#include "hal_callbacks.h"

#include "main.h"

#include "app_board_slave.h"
#include "app_debug.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

static uint8_t s_cli_rx_byte;
static uint8_t s_uart1_rx_byte;

void hal_callbacks_init(void)
{
    (void)HAL_UART_Receive_IT(&huart2, &s_cli_rx_byte, 1U);
    (void)HAL_UART_Receive_IT(&huart1, &s_uart1_rx_byte, 1U);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == NULL)
    {
        return;
    }

    if (huart->Instance == USART2)
    {
        app_debug_rx_byte_from_isr(s_cli_rx_byte);
        (void)HAL_UART_Receive_IT(&huart2, &s_cli_rx_byte, 1U);
    }
    else if (huart->Instance == USART1)
    {
        app_board_slave_rx_byte_from_isr(1U, s_uart1_rx_byte);
        (void)HAL_UART_Receive_IT(&huart1, &s_uart1_rx_byte, 1U);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart == NULL)
    {
        return;
    }

    if (huart->Instance == USART2)
    {
        (void)HAL_UART_Receive_IT(&huart2, &s_cli_rx_byte, 1U);
    }
    else if (huart->Instance == USART1)
    {
        (void)HAL_UART_Receive_IT(&huart1, &s_uart1_rx_byte, 1U);
    }
}
