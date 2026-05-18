#include "board_devices.h"

#if BOARD_HAS_UART1
extern UART_HandleTypeDef huart1;
#endif

#if BOARD_HAS_UART2
extern UART_HandleTypeDef huart2;
#endif

#if BOARD_HAS_SPI1
extern SPI_HandleTypeDef hspi1;
#endif

#if BOARD_HAS_SPI2
extern SPI_HandleTypeDef hspi2;
#endif

#if BOARD_HAS_I2C1
extern I2C_HandleTypeDef hi2c1;
#endif

#if BOARD_HAS_I2C2
extern I2C_HandleTypeDef hi2c2;
#endif

#if BOARD_HAS_CAN1
/*
 * CubeMX names the single CAN handle on the current STM32F1 board "hcan".
 * On a board with CAN1/CAN2 handles, update this board-specific mapping only.
 */
extern CAN_HandleTypeDef hcan;
#endif

#if BOARD_HAS_CAN2
extern CAN_HandleTypeDef hcan2;
#endif

UART_HandleTypeDef *board_devices_get_uart(uint8_t instance)
{
    switch (instance)
    {
#if BOARD_HAS_UART1
        case 1U:
            return &huart1;
#endif

#if BOARD_HAS_UART2
        case 2U:
            return &huart2;
#endif

        default:
            return 0;
    }
}

#if BOARD_HAS_ANY_SPI
SPI_HandleTypeDef *board_devices_get_spi(uint8_t instance)
{
    switch (instance)
    {
#if BOARD_HAS_SPI1
        case 1U:
            return &hspi1;
#endif

#if BOARD_HAS_SPI2
        case 2U:
            return &hspi2;
#endif

        default:
            return 0;
    }
}
#endif

#if BOARD_HAS_ANY_I2C
I2C_HandleTypeDef *board_devices_get_i2c(uint8_t instance)
{
    switch (instance)
    {
#if BOARD_HAS_I2C1
        case 1U:
            return &hi2c1;
#endif

#if BOARD_HAS_I2C2
        case 2U:
            return &hi2c2;
#endif

        default:
            return 0;
    }
}
#endif

#if BOARD_HAS_ANY_CAN
CAN_HandleTypeDef *board_devices_get_can(uint8_t instance)
{
    switch (instance)
    {
#if BOARD_HAS_CAN1
        case 1U:
            return &hcan;
#endif

#if BOARD_HAS_CAN2
        case 2U:
            return &hcan2;
#endif

        default:
            return 0;
    }
}
#endif

uint8_t board_devices_uart_instance_from_handle(const UART_HandleTypeDef *uart)
{
#if BOARD_HAS_UART1
    if (uart == &huart1)
    {
        return 1U;
    }
#endif

#if BOARD_HAS_UART2
    if (uart == &huart2)
    {
        return 2U;
    }
#endif

    return 0U;
}

#if BOARD_HAS_ANY_SPI
uint8_t board_devices_spi_instance_from_handle(const SPI_HandleTypeDef *spi)
{
#if BOARD_HAS_SPI1
    if (spi == &hspi1)
    {
        return 1U;
    }
#endif

#if BOARD_HAS_SPI2
    if (spi == &hspi2)
    {
        return 2U;
    }
#endif

    return 0U;
}
#endif

#if BOARD_HAS_ANY_I2C
uint8_t board_devices_i2c_instance_from_handle(const I2C_HandleTypeDef *i2c)
{
#if BOARD_HAS_I2C1
    if (i2c == &hi2c1)
    {
        return 1U;
    }
#endif

#if BOARD_HAS_I2C2
    if (i2c == &hi2c2)
    {
        return 2U;
    }
#endif

    return 0U;
}
#endif

#if BOARD_HAS_ANY_CAN
uint8_t board_devices_can_instance_from_handle(const CAN_HandleTypeDef *can)
{
#if BOARD_HAS_CAN1
    if (can == &hcan)
    {
        return 1U;
    }
#endif

#if BOARD_HAS_CAN2
    if (can == &hcan2)
    {
        return 2U;
    }
#endif

    return 0U;
}
#endif
