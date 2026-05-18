#include "board_service.h"

#include "board_devices.h"
#include "gpio_port.h"
#include "i2c_port.h"
#include "param.h"
#include "slave_link_port.h"
#include "spi_port.h"
#include "uart_port.h"

typedef enum
{
    BOARD_SERVICE_TRANSPORT_UART = 0,
    BOARD_SERVICE_TRANSPORT_SPI,
    BOARD_SERVICE_TRANSPORT_I2C
} board_service_transport_t;

typedef enum
{
    BOARD_SERVICE_LINK_ROLE_MASTER = 0,
    BOARD_SERVICE_LINK_ROLE_SLAVE
} board_service_link_role_t;

/*
 * Board mapping: change these definitions when moving to another board.
 */
#define BOARD_SERVICE_CONSOLE_WRITE_TIMEOUT_MS   100U

#define BOARD_SERVICE_SLAVE_LINK_TRANSPORT       BOARD_SERVICE_TRANSPORT_UART
#define BOARD_SERVICE_SLAVE_LINK_INSTANCE        1U
#define BOARD_SERVICE_SLAVE_LINK_ROLE            BOARD_SERVICE_LINK_ROLE_MASTER
#define BOARD_SERVICE_SLAVE_LINK_I2C_ADDR7       0x00U

#define BOARD_SERVICE_STATUS_LED_ID              0U
#define BOARD_SERVICE_STATUS_LED_PORT            GPIOC
#define BOARD_SERVICE_STATUS_LED_PIN             GPIO_PIN_13
#define BOARD_SERVICE_STATUS_LED_ACTIVE_LOW      1U

static uint8_t board_service_transport_is_enabled(board_service_transport_t transport,
                                                  uint8_t instance);
static uint8_t board_service_transport_is_slave(board_service_transport_t transport,
                                                uint8_t instance);
static uint8_t board_service_transport_is_selected_slave_link(
    board_service_transport_t transport,
    uint8_t instance);
static uint8_t board_service_transport_write(board_service_transport_t transport,
                                             uint8_t instance,
                                             const uint8_t *data,
                                             uint8_t len,
                                             uint32_t timeout_ms);
static uart_protocol_t board_service_uart_protocol(uint8_t instance);
static uint8_t board_service_find_uart_protocol(uart_protocol_t protocol);
static uint8_t board_service_status_led_id_is_valid(uint8_t led_id);
static GPIO_PinState board_service_status_led_on_state(void);
static GPIO_PinState board_service_status_led_off_state(void);

void board_service_init(void)
{
    /* Board-level services are currently configured statically in param.c. */
}

void board_service_console_init(void)
{
    /*
     * Transport initialization is still handled by generated startup code.
     * This function exists so Application does not need to know how the
     * console is mapped on this board.
     */
}

void board_service_console_write(const uint8_t *data, uint16_t len)
{
    uint8_t instance;

    if ((data == 0) || (len == 0U) || (len > 255U))
    {
        return;
    }

    instance = board_service_find_uart_protocol(UART_PROTOCOL_DEBUG);

    if (instance != 0U)
    {
        (void)uart_port_write(instance,
                              data,
                              (uint8_t)len,
                              BOARD_SERVICE_CONSOLE_WRITE_TIMEOUT_MS);
    }
}

uint8_t board_service_console_uses_uart(uint8_t instance)
{
    return (uint8_t)(board_service_uart_protocol(instance) == UART_PROTOCOL_DEBUG);
}

uint8_t board_service_gps_uses_uart(uint8_t instance)
{
    return (uint8_t)(board_service_uart_protocol(instance) == UART_PROTOCOL_GPS);
}

void board_service_status_led_init(void)
{
    gpio_port_write(BOARD_SERVICE_STATUS_LED_PORT,
                    BOARD_SERVICE_STATUS_LED_PIN,
                    board_service_status_led_off_state());
}

void board_service_status_led_on(uint8_t led_id)
{
    if (board_service_status_led_id_is_valid(led_id) != 0U)
    {
        gpio_port_write(BOARD_SERVICE_STATUS_LED_PORT,
                        BOARD_SERVICE_STATUS_LED_PIN,
                        board_service_status_led_on_state());
    }
}

void board_service_status_led_off(uint8_t led_id)
{
    if (board_service_status_led_id_is_valid(led_id) != 0U)
    {
        gpio_port_write(BOARD_SERVICE_STATUS_LED_PORT,
                        BOARD_SERVICE_STATUS_LED_PIN,
                        board_service_status_led_off_state());
    }
}

void board_service_status_led_toggle(uint8_t led_id)
{
    if (board_service_status_led_id_is_valid(led_id) != 0U)
    {
        gpio_port_toggle(BOARD_SERVICE_STATUS_LED_PORT,
                         BOARD_SERVICE_STATUS_LED_PIN);
    }
}

void board_service_status_led_toggle_default(void)
{
    board_service_status_led_toggle(BOARD_SERVICE_STATUS_LED_ID);
}

uint8_t board_service_slave_link_is_active(void)
{
    return (uint8_t)((board_service_transport_is_enabled(
                          BOARD_SERVICE_SLAVE_LINK_TRANSPORT,
                          BOARD_SERVICE_SLAVE_LINK_INSTANCE) != 0U) &&
                     (board_service_transport_is_slave(
                          BOARD_SERVICE_SLAVE_LINK_TRANSPORT,
                          BOARD_SERVICE_SLAVE_LINK_INSTANCE) != 0U));
}

uint8_t board_service_slave_link_write(const uint8_t *data,
                                       uint8_t len,
                                       uint32_t timeout_ms)
{
    if (board_service_slave_link_is_active() == 0U)
    {
        return 0U;
    }

    return board_service_transport_write(BOARD_SERVICE_SLAVE_LINK_TRANSPORT,
                                         BOARD_SERVICE_SLAVE_LINK_INSTANCE,
                                         data,
                                         len,
                                         timeout_ms);
}

uint8_t board_service_slave_link_poll_rx(uint32_t timeout_ms)
{
    if ((board_service_slave_link_is_active() == 0U) ||
        (BOARD_SERVICE_SLAVE_LINK_ROLE != BOARD_SERVICE_LINK_ROLE_MASTER))
    {
        return 1U;
    }

    switch (BOARD_SERVICE_SLAVE_LINK_TRANSPORT)
    {
        case BOARD_SERVICE_TRANSPORT_SPI:
            return slave_link_port_spi_master_poll_rx(BOARD_SERVICE_SLAVE_LINK_INSTANCE,
                                                      timeout_ms);

        case BOARD_SERVICE_TRANSPORT_I2C:
            return slave_link_port_i2c_master_poll_rx(BOARD_SERVICE_SLAVE_LINK_INSTANCE,
                                                      BOARD_SERVICE_SLAVE_LINK_I2C_ADDR7,
                                                      timeout_ms);

        case BOARD_SERVICE_TRANSPORT_UART:
        default:
            return 1U;
    }
}

uint8_t board_service_slave_link_start_rx(void)
{
    if ((board_service_slave_link_is_active() == 0U) ||
        (BOARD_SERVICE_SLAVE_LINK_ROLE != BOARD_SERVICE_LINK_ROLE_SLAVE))
    {
        return 1U;
    }

    switch (BOARD_SERVICE_SLAVE_LINK_TRANSPORT)
    {
        case BOARD_SERVICE_TRANSPORT_SPI:
            return slave_link_port_spi_slave_start(BOARD_SERVICE_SLAVE_LINK_INSTANCE);

        case BOARD_SERVICE_TRANSPORT_I2C:
            return slave_link_port_i2c_slave_start(BOARD_SERVICE_SLAVE_LINK_INSTANCE);

        case BOARD_SERVICE_TRANSPORT_UART:
        default:
            return 1U;
    }
}

uint8_t board_service_slave_link_uses_uart(uint8_t instance)
{
    return (uint8_t)((BOARD_SERVICE_SLAVE_LINK_TRANSPORT ==
                      BOARD_SERVICE_TRANSPORT_UART) &&
                     (BOARD_SERVICE_SLAVE_LINK_INSTANCE == instance) &&
                     (board_service_slave_link_is_active() != 0U));
}

uint8_t board_service_slave_link_uses_spi(uint8_t instance)
{
    return (uint8_t)((BOARD_SERVICE_SLAVE_LINK_TRANSPORT ==
                      BOARD_SERVICE_TRANSPORT_SPI) &&
                     (BOARD_SERVICE_SLAVE_LINK_INSTANCE == instance) &&
                     (BOARD_SERVICE_SLAVE_LINK_ROLE == BOARD_SERVICE_LINK_ROLE_SLAVE) &&
                     (board_service_slave_link_is_active() != 0U));
}

uint8_t board_service_slave_link_uses_i2c(uint8_t instance)
{
    return (uint8_t)((BOARD_SERVICE_SLAVE_LINK_TRANSPORT ==
                      BOARD_SERVICE_TRANSPORT_I2C) &&
                     (BOARD_SERVICE_SLAVE_LINK_INSTANCE == instance) &&
                     (BOARD_SERVICE_SLAVE_LINK_ROLE == BOARD_SERVICE_LINK_ROLE_SLAVE) &&
                     (board_service_slave_link_is_active() != 0U));
}

static uint8_t board_service_transport_is_enabled(board_service_transport_t transport,
                                                  uint8_t instance)
{
    switch (transport)
    {
        case BOARD_SERVICE_TRANSPORT_UART:
            return uart_port_is_enabled(instance);

        case BOARD_SERVICE_TRANSPORT_SPI:
            return spi_port_is_enabled(instance);

        case BOARD_SERVICE_TRANSPORT_I2C:
            return i2c_port_is_enabled(instance);

        default:
            return 0U;
    }
}

static uint8_t board_service_transport_is_slave(board_service_transport_t transport,
                                                uint8_t instance)
{
    switch (transport)
    {
        case BOARD_SERVICE_TRANSPORT_UART:
            return (uint8_t)(board_service_uart_protocol(instance) ==
                             UART_PROTOCOL_SLAVE);

        case BOARD_SERVICE_TRANSPORT_SPI:
            if (instance == 1U)
            {
                return (uint8_t)(param_get(PARAM_SPI1_PROTOCOL) ==
                                 BUS_PROTOCOL_SLAVE);
            }

            if (instance == 2U)
            {
                return (uint8_t)(param_get(PARAM_SPI2_PROTOCOL) ==
                                 BUS_PROTOCOL_SLAVE);
            }

            return 0U;

        case BOARD_SERVICE_TRANSPORT_I2C:
            if (instance == 1U)
            {
                return (uint8_t)(param_get(PARAM_I2C1_PROTOCOL) ==
                                 BUS_PROTOCOL_SLAVE);
            }

            if (instance == 2U)
            {
                return (uint8_t)(param_get(PARAM_I2C2_PROTOCOL) ==
                                 BUS_PROTOCOL_SLAVE);
            }

            return 0U;

        default:
            return 0U;
    }
}

static uint8_t board_service_transport_write(board_service_transport_t transport,
                                             uint8_t instance,
                                             const uint8_t *data,
                                             uint8_t len,
                                             uint32_t timeout_ms)
{
    switch (transport)
    {
        case BOARD_SERVICE_TRANSPORT_UART:
            if (board_service_transport_is_selected_slave_link(transport, instance) != 0U)
            {
                return slave_link_port_uart_write(instance, data, len, timeout_ms);
            }

            return uart_port_write(instance, data, len, timeout_ms);

        case BOARD_SERVICE_TRANSPORT_SPI:
            if ((board_service_transport_is_selected_slave_link(transport, instance) != 0U) &&
                (BOARD_SERVICE_SLAVE_LINK_ROLE == BOARD_SERVICE_LINK_ROLE_SLAVE))
            {
                return slave_link_port_stage_tx_frame(data, len);
            }

            if (board_service_transport_is_selected_slave_link(transport, instance) != 0U)
            {
                return slave_link_port_spi_master_write(instance, data, len, timeout_ms);
            }

            return spi_port_write(instance, data, len, timeout_ms);

        case BOARD_SERVICE_TRANSPORT_I2C:
            if ((board_service_transport_is_selected_slave_link(transport, instance) != 0U) &&
                (BOARD_SERVICE_SLAVE_LINK_ROLE == BOARD_SERVICE_LINK_ROLE_SLAVE))
            {
                return slave_link_port_stage_tx_frame(data, len);
            }

            if (board_service_transport_is_selected_slave_link(transport, instance) != 0U)
            {
                return slave_link_port_i2c_master_write(instance,
                                                        BOARD_SERVICE_SLAVE_LINK_I2C_ADDR7,
                                                        data,
                                                        len,
                                                        timeout_ms);
            }

            return 0U;

        default:
            return 0U;
    }
}

static uint8_t board_service_transport_is_selected_slave_link(
    board_service_transport_t transport,
    uint8_t instance)
{
    return (uint8_t)((BOARD_SERVICE_SLAVE_LINK_TRANSPORT == transport) &&
                     (BOARD_SERVICE_SLAVE_LINK_INSTANCE == instance));
}

static uint8_t board_service_status_led_id_is_valid(uint8_t led_id)
{
    return (uint8_t)(led_id == BOARD_SERVICE_STATUS_LED_ID);
}

static GPIO_PinState board_service_status_led_on_state(void)
{
    return (BOARD_SERVICE_STATUS_LED_ACTIVE_LOW != 0U) ?
           GPIO_PIN_RESET :
           GPIO_PIN_SET;
}

static GPIO_PinState board_service_status_led_off_state(void)
{
    return (BOARD_SERVICE_STATUS_LED_ACTIVE_LOW != 0U) ?
           GPIO_PIN_SET :
           GPIO_PIN_RESET;
}

static uart_protocol_t board_service_uart_protocol(uint8_t instance)
{
    if (instance == 1U)
    {
        return (uart_protocol_t)param_get(PARAM_UART1_PROTOCOL);
    }

    if (instance == 2U)
    {
        return (uart_protocol_t)param_get(PARAM_UART2_PROTOCOL);
    }

    return UART_PROTOCOL_DISABLED;
}

static uint8_t board_service_find_uart_protocol(uart_protocol_t protocol)
{
    uint8_t instance;

    for (instance = 1U; instance <= BOARD_UART_INSTANCE_MAX; instance++)
    {
        if ((uart_port_is_enabled(instance) != 0U) &&
            (board_service_uart_protocol(instance) == protocol))
        {
            return instance;
        }
    }

    return 0U;
}
