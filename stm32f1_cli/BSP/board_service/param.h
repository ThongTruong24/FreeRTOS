#ifndef PARAM_H
#define PARAM_H

#include <stdint.h>

typedef enum {
    PARAM_UART1_PROTOCOL = 0,
    PARAM_UART2_PROTOCOL,
    PARAM_SPI1_PROTOCOL,
    PARAM_SPI2_PROTOCOL,
    PARAM_I2C1_PROTOCOL,
    PARAM_I2C2_PROTOCOL,
    PARAM_PROTO_TIMEOUT_MS,
    PARAM_LED_DEFAULT_HZ,
    PARAM_LED_DEFAULT_COUNT,
    PARAM_COUNT
} param_id_t;

typedef enum {
    UART_PROTOCOL_DISABLED = 0,
    UART_PROTOCOL_DEBUG = 1,
    UART_PROTOCOL_SLAVE = 2,
    UART_PROTOCOL_GPS = 3
} uart_protocol_t;

typedef enum {
    BUS_PROTOCOL_DISABLED = 0,
    BUS_PROTOCOL_SLAVE = 1
} slave_bus_protocol_t;

const char *param_name(param_id_t id);
int32_t param_get(param_id_t id);

#endif
