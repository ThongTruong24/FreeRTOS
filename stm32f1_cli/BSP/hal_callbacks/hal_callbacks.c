#include "hal_callbacks.h"

#include "main.h"

#include "app_debug.h"
#include "app_gps.h"
#include "app_task.h"
#include "board_devices.h"
#include "board_service.h"
#include "can_port.h"
#include "i2c_port.h"
#include "slave_link_port.h"
#include "spi_port.h"
#include "uart_port.h"

static uint8_t s_uart_rx_byte[BOARD_UART_INSTANCE_MAX + 1U];

static uint8_t *hal_callbacks_uart_rx_byte_buffer(uint8_t instance);
#if BOARD_HAS_ANY_CAN
static uint32_t s_can_last_error[BOARD_CAN_INSTANCE_MAX + 1U];
static void hal_callbacks_post_can_status_from_isr(CAN_HandleTypeDef *hcan,
                                                   app_can_status_event_t event);
#endif

void hal_callbacks_init(void)
{
    uint8_t instance;

    for (instance = 1U; instance <= BOARD_UART_INSTANCE_MAX; instance++)
    {
        if (board_devices_get_uart(instance) != 0)
        {
            (void)uart_port_start_receive_it(instance,
                                             &s_uart_rx_byte[instance],
                                             1U);
        }
    }

    (void)board_service_slave_link_start_rx();
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    uint8_t instance;
    uint8_t *rx_byte;

    if (huart == NULL)
    {
        return;
    }

    instance = uart_port_instance_from_handle(huart);
    rx_byte = hal_callbacks_uart_rx_byte_buffer(instance);

    if (rx_byte == NULL)
    {
        return;
    }

    if (board_service_console_uses_uart(instance) != 0U)
    {
        app_debug_rx_byte_from_isr(*rx_byte);
    }

    if (board_service_slave_link_uses_uart(instance) != 0U)
    {
        slave_link_port_uart_rx_byte_from_isr(*rx_byte);
    }

    if (board_service_gps_uses_uart(instance) != 0U)
    {
        app_gps_rx_byte_from_isr(*rx_byte);
    }

    (void)uart_port_start_receive_it(instance, rx_byte, 1U);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    uint8_t instance;
    uint8_t *rx_byte;

    if (huart == NULL)
    {
        return;
    }

    instance = uart_port_instance_from_handle(huart);
    rx_byte = hal_callbacks_uart_rx_byte_buffer(instance);

    if (rx_byte != NULL)
    {
        (void)uart_port_start_receive_it(instance, rx_byte, 1U);
    }
}

static uint8_t *hal_callbacks_uart_rx_byte_buffer(uint8_t instance)
{
    if ((instance == 0U) ||
        (instance > BOARD_UART_INSTANCE_MAX) ||
        (board_devices_get_uart(instance) == 0))
    {
        return NULL;
    }

    return &s_uart_rx_byte[instance];
}

#if BOARD_HAS_ANY_CAN
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan)
{
    hal_callbacks_post_can_status_from_isr(hcan, CAN_STATUS_TX_COMPLETE);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan)
{
    hal_callbacks_post_can_status_from_isr(hcan, CAN_STATUS_TX_COMPLETE);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan)
{
    hal_callbacks_post_can_status_from_isr(hcan, CAN_STATUS_TX_COMPLETE);
}

void HAL_CAN_TxMailbox0AbortCallback(CAN_HandleTypeDef *hcan)
{
    hal_callbacks_post_can_status_from_isr(hcan, CAN_STATUS_TX_ABORTED);
}

void HAL_CAN_TxMailbox1AbortCallback(CAN_HandleTypeDef *hcan)
{
    hal_callbacks_post_can_status_from_isr(hcan, CAN_STATUS_TX_ABORTED);
}

void HAL_CAN_TxMailbox2AbortCallback(CAN_HandleTypeDef *hcan)
{
    hal_callbacks_post_can_status_from_isr(hcan, CAN_STATUS_TX_ABORTED);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
    hal_callbacks_post_can_status_from_isr(hcan, CAN_STATUS_ERROR);
}

static void hal_callbacks_post_can_status_from_isr(CAN_HandleTypeDef *hcan,
                                                   app_can_status_event_t event)
{
    uint8_t instance;
    uint32_t error_code = 0U;
    BaseType_t higher_priority_task_woken = pdFALSE;
    app_msg_t msg = {0};

    if (hcan == NULL)
    {
        return;
    }

    instance = can_port_instance_from_handle(hcan);

    if (board_service_can_uses_debug(instance) == 0U)
    {
        return;
    }

    if (event == CAN_STATUS_ERROR)
    {
        error_code = board_service_can_take_error(instance);

        if ((error_code == BOARD_SERVICE_CAN_ERROR_NONE) ||
            (error_code == s_can_last_error[instance]))
        {
            return;
        }

        s_can_last_error[instance] = error_code;
    }
    else if (event == CAN_STATUS_TX_COMPLETE)
    {
        s_can_last_error[instance] = BOARD_SERVICE_CAN_ERROR_NONE;
    }

    msg.id = APP_MSG_CAN_STATUS;
    msg.len = sizeof(msg.payload.can_status);
    msg.payload.can_status.instance = instance;
    msg.payload.can_status.event = (uint8_t)event;
    msg.payload.can_status.error_code = error_code;

    (void)app_task_post_from_isr(&msg, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}
#endif

#if BOARD_HAS_ANY_SPI
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t instance;

    if (hspi == NULL)
    {
        return;
    }

    instance = spi_port_instance_from_handle(hspi);

    if (board_service_slave_link_uses_spi(instance) != 0U)
    {
        slave_link_port_spi_slave_transfer_complete_from_isr(instance);
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    uint8_t instance;

    if (hspi == NULL)
    {
        return;
    }

    instance = spi_port_instance_from_handle(hspi);

    if (board_service_slave_link_uses_spi(instance) != 0U)
    {
        slave_link_port_spi_slave_error_from_isr(instance);
    }
}
#endif

#if BOARD_HAS_ANY_I2C
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c,
                          uint8_t transfer_direction,
                          uint16_t addr_match_code)
{
    uint8_t instance;

    (void)addr_match_code;

    if (hi2c == NULL)
    {
        return;
    }

    instance = i2c_port_instance_from_handle(hi2c);

    if (board_service_slave_link_uses_i2c(instance) != 0U)
    {
        slave_link_port_i2c_slave_addr_from_isr(instance, transfer_direction);
    }
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t instance;

    if (hi2c == NULL)
    {
        return;
    }

    instance = i2c_port_instance_from_handle(hi2c);

    if (board_service_slave_link_uses_i2c(instance) != 0U)
    {
        slave_link_port_i2c_slave_rx_complete_from_isr(instance);
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t instance;

    if (hi2c == NULL)
    {
        return;
    }

    instance = i2c_port_instance_from_handle(hi2c);

    if (board_service_slave_link_uses_i2c(instance) != 0U)
    {
        slave_link_port_i2c_slave_tx_complete_from_isr(instance);
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    uint8_t instance;

    if (hi2c == NULL)
    {
        return;
    }

    instance = i2c_port_instance_from_handle(hi2c);

    if (board_service_slave_link_uses_i2c(instance) != 0U)
    {
        slave_link_port_i2c_slave_error_from_isr(instance);
    }
}
#endif
