#ifndef UART_PORT_H
#define UART_PORT_H

#include <stdint.h>

#include "main.h"

uint8_t uart_port_is_enabled(uint8_t instance);
uint8_t uart_port_write(uint8_t instance, const uint8_t *data, uint8_t len, uint32_t timeout_ms);
uint8_t uart_port_start_receive_it(uint8_t instance, uint8_t *data, uint8_t len);
uint8_t uart_port_instance_from_handle(const UART_HandleTypeDef *uart);
uint8_t uart_port_init(uint8_t instance);
uint8_t uart_port_deinit(uint8_t instance);

#endif
