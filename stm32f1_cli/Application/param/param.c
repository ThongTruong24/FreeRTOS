#include "param.h"
#include <string.h>

typedef struct {
    const char *name;
    int32_t default_value;
    int32_t min_value;
    int32_t max_value;
    int32_t value;
} param_entry_t;

static param_entry_t g_params[PARAM_COUNT] = {
    [PARAM_UART1_EN] = {"UART1_EN", 1, 0, 1, 1},
    [PARAM_UART1_PROTOCOL] = {"UART1_PROTOCOL", UART1_PROTOCOL_SLAVE_CONTROL,
                              UART1_PROTOCOL_SLAVE_CONTROL, UART1_PROTOCOL_GPS_BN220,
                              UART1_PROTOCOL_SLAVE_CONTROL},
    [PARAM_SPI1_EN] = {"SPI1_EN", 0, 0, 1, 0},
    [PARAM_SPI2_EN] = {"SPI2_EN", 0, 0, 1, 0},
    [PARAM_I2C1_EN] = {"I2C1_EN", 0, 0, 1, 0},
    [PARAM_I2C2_EN] = {"I2C2_EN", 0, 0, 1, 0},
    [PARAM_PROTO_TIMEOUT_MS] = {"PROTO_TIMEOUT_MS", 100, 1, 5000, 100},
    [PARAM_LED_DEFAULT_HZ] = {"LED_DEFAULT_HZ", 1, 1, 100, 1},
    [PARAM_LED_DEFAULT_COUNT] = {"LED_DEFAULT_COUNT", 0, 0, 32767, 0},
};

void param_init(void)
{
    uint8_t i;

    for (i = 0; i < PARAM_COUNT; i++) {
        g_params[i].value = g_params[i].default_value;
    }
}

uint8_t param_find(const char *name, param_id_t *id)
{
    uint8_t i;

    if ((name == 0) || (id == 0)) {
        return 0;
    }

    for (i = 0; i < PARAM_COUNT; i++) {
        if (strcmp(name, g_params[i].name) == 0) {
            *id = (param_id_t)i;
            return 1;
        }
    }

    return 0;
}

const char *param_name(param_id_t id)
{
    if (id >= PARAM_COUNT) {
        return "";
    }

    return g_params[id].name;
}

int32_t param_get(param_id_t id)
{
    if (id >= PARAM_COUNT) {
        return 0;
    }

    return g_params[id].value;
}

uint8_t param_set(param_id_t id, int32_t value)
{
    if (id >= PARAM_COUNT) {
        return 0;
    }

    if ((value < g_params[id].min_value) || (value > g_params[id].max_value)) {
        return 0;
    }

    g_params[id].value = value;
    return 1;
}

uint8_t param_set_by_name(const char *name, int32_t value)
{
    param_id_t id;

    if (param_find(name, &id) == 0U) {
        return 0;
    }

    return param_set(id, value);
}

uint8_t param_get_by_name(const char *name, int32_t *value)
{
    param_id_t id;

    if ((value == 0) || (param_find(name, &id) == 0U)) {
        return 0;
    }

    *value = param_get(id);
    return 1;
}
