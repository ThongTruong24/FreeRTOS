#include "param.h"

typedef struct
{
    const char *name;
    int32_t value;
} param_entry_t;

static const param_entry_t g_params[PARAM_COUNT] = {
    [PARAM_UART1_PROTOCOL] = {"UART1_PROTOCOL", UART_PROTOCOL_SLAVE},
    [PARAM_UART2_PROTOCOL] = {"UART2_PROTOCOL", UART_PROTOCOL_DEBUG},
    [PARAM_SPI1_PROTOCOL] = {"SPI1_PROTOCOL", BUS_PROTOCOL_DISABLED},
    [PARAM_SPI2_PROTOCOL] = {"SPI2_PROTOCOL", BUS_PROTOCOL_DISABLED},
    [PARAM_I2C1_PROTOCOL] = {"I2C1_PROTOCOL", BUS_PROTOCOL_DISABLED},
    [PARAM_I2C2_PROTOCOL] = {"I2C2_PROTOCOL", BUS_PROTOCOL_DISABLED},
    [PARAM_CAN1_PROTOCOL] = {"CAN1_PROTOCOL", CAN_PROTOCOL_DEBUG},
    [PARAM_CAN2_PROTOCOL] = {"CAN2_PROTOCOL", CAN_PROTOCOL_DISABLED},
    [PARAM_PROTO_TIMEOUT_MS] = {"PROTO_TIMEOUT_MS", 100},
    [PARAM_LED_DEFAULT_HZ] = {"LED_DEFAULT_HZ", 1},
    [PARAM_LED_DEFAULT_COUNT] = {"LED_DEFAULT_COUNT", 0},
};

static uint8_t param_id_is_valid(param_id_t id)
{
    return (uint8_t)(id < PARAM_COUNT);
}

const char *param_name(param_id_t id)
{
    if (param_id_is_valid(id) == 0U)
    {
        return "";
    }

    return g_params[id].name;
}

int32_t param_get(param_id_t id)
{
    if (param_id_is_valid(id) == 0U)
    {
        return 0;
    }

    return g_params[id].value;
}
