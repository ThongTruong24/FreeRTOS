#include "uart_port.h"

#include "board_devices.h"

static UART_HandleTypeDef *uart_port_get_handle(uint8_t instance)
{
    return board_devices_get_uart(instance);
}

uint8_t uart_port_is_enabled(uint8_t instance)
{
    return (uint8_t)(uart_port_get_handle(instance) != 0);
}

uint8_t uart_port_write(uint8_t instance, const uint8_t *data, uint8_t len, uint32_t timeout_ms)
{
    UART_HandleTypeDef *uart = uart_port_get_handle(instance);

    if ((uart == 0) || (data == 0) || (len == 0U) ||
        (uart_port_is_enabled(instance) == 0U)) {
        return 0;
    }

    return (HAL_UART_Transmit(uart, (uint8_t *)data, len, timeout_ms) == HAL_OK) ? 1U : 0U;
}

uint8_t uart_port_start_receive_it(uint8_t instance, uint8_t *data, uint8_t len)
{
    UART_HandleTypeDef *uart = uart_port_get_handle(instance);

    if ((uart == 0) || (data == 0) || (len == 0U) ||
        (uart_port_is_enabled(instance) == 0U)) {
        return 0U;
    }

    return (HAL_UART_Receive_IT(uart, data, len) == HAL_OK) ? 1U : 0U;
}

uint8_t uart_port_instance_from_handle(const UART_HandleTypeDef *uart)
{
    return board_devices_uart_instance_from_handle(uart);
}

uint8_t uart_port_init(uint8_t instance)
{
    UART_HandleTypeDef *uart = uart_port_get_handle(instance);

    return ((uart != 0) && (HAL_UART_Init(uart) == HAL_OK)) ? 1U : 0U;
}

uint8_t uart_port_deinit(uint8_t instance)
{
    UART_HandleTypeDef *uart = uart_port_get_handle(instance);

    return ((uart != 0) && (HAL_UART_DeInit(uart) == HAL_OK)) ? 1U : 0U;
}
