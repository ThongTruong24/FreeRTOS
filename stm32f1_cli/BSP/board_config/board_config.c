#include "board_config.h"
#include "i2c_port.h"
#include "param.h"
#include "spi_port.h"
#include "uart_port.h"

static uint8_t apply_uart(uint8_t instance, board_config_action_t action)
{
    if ((instance == 1U) &&
        (param_get(PARAM_UART1_EN) == (int32_t)(action == BOARD_CONFIG_ENABLE))) {
        return 1;
    }

    return (action == BOARD_CONFIG_ENABLE) ?
        uart_port_init(instance) :
        uart_port_deinit(instance);
}

static uint8_t apply_spi(uint8_t instance, board_config_action_t action)
{
    return (action == BOARD_CONFIG_ENABLE) ?
        spi_port_init(instance) :
        spi_port_deinit(instance);
}

static uint8_t apply_i2c(uint8_t instance, board_config_action_t action)
{
    return (action == BOARD_CONFIG_ENABLE) ?
        i2c_port_init(instance) :
        i2c_port_deinit(instance);
}

uint8_t board_config_apply(const app_board_config_msg_t *msg)
{
    uint8_t ok;

    if (msg == 0) {
        return 0;
    }

    switch (msg->bus) {
    case BOARD_BUS_I2C:
        ok = apply_i2c(msg->instance, msg->action);
        if ((ok != 0U) && (msg->instance == 1U)) {
            (void)param_set(PARAM_I2C1_EN, msg->action == BOARD_CONFIG_ENABLE);
        } else if ((ok != 0U) && (msg->instance == 2U)) {
            (void)param_set(PARAM_I2C2_EN, msg->action == BOARD_CONFIG_ENABLE);
        }
        return ok;
    case BOARD_BUS_SPI:
        ok = apply_spi(msg->instance, msg->action);
        if ((ok != 0U) && (msg->instance == 1U)) {
            (void)param_set(PARAM_SPI1_EN, msg->action == BOARD_CONFIG_ENABLE);
        } else if ((ok != 0U) && (msg->instance == 2U)) {
            (void)param_set(PARAM_SPI2_EN, msg->action == BOARD_CONFIG_ENABLE);
        }
        return ok;
    case BOARD_BUS_UART:
        ok = apply_uart(msg->instance, msg->action);
        if ((ok != 0U) && (msg->instance == 1U)) {
            (void)param_set(PARAM_UART1_EN, msg->action == BOARD_CONFIG_ENABLE);
        }
        return ok;
    default:
        return 0;
    }
}

const char *board_config_bus_name(board_bus_t bus)
{
    switch (bus) {
    case BOARD_BUS_I2C:
        return "i2c";
    case BOARD_BUS_SPI:
        return "spi";
    case BOARD_BUS_UART:
        return "uart";
    default:
        return "unknown";
    }
}
