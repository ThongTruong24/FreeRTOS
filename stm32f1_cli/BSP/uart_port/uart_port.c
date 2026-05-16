#include "uart_port.h"
#include "main.h"
#include "param.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

static UART_HandleTypeDef *uart_port_get_handle(uint8_t instance)
{
    if (instance == 1U) {
        return &huart1;
    }

    if (instance == 2U) {
        return &huart2;
    }

    return 0;
}

uint8_t uart_port_is_enabled(uint8_t instance)
{
    if (instance == 1U) {
        return (param_get(PARAM_UART1_EN) != 0) ? 1U : 0U;
    }

    if (instance == 2U) {
        return 1U;
    }

    return 0;
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
