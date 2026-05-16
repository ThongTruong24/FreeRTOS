#ifndef PARAM_H
#define PARAM_H

#include <stdint.h>

typedef enum {
    PARAM_UART1_EN = 0,
    PARAM_UART1_PROTOCOL,
    PARAM_SPI1_EN,
    PARAM_SPI2_EN,
    PARAM_I2C1_EN,
    PARAM_I2C2_EN,
    PARAM_PROTO_TIMEOUT_MS,
    PARAM_LED_DEFAULT_HZ,
    PARAM_LED_DEFAULT_COUNT,
    PARAM_COUNT
} param_id_t;

typedef enum {
    UART1_PROTOCOL_SLAVE_CONTROL = 0,
    UART1_PROTOCOL_GPS_BN220 = 1
} uart1_protocol_t;

void param_init(void);
uint8_t param_find(const char *name, param_id_t *id);
const char *param_name(param_id_t id);
int32_t param_get(param_id_t id);
uint8_t param_set(param_id_t id, int32_t value);
uint8_t param_set_by_name(const char *name, int32_t value);
uint8_t param_get_by_name(const char *name, int32_t *value);

#endif
