#ifndef BOARD_DEVICES_H
#define BOARD_DEVICES_H

#include <stdint.h>

#include "main.h"

/*
 * Board capability table.
 *
 * Keep these flags aligned with the peripherals that CubeMX generated for the
 * current board. When a peripheral is not generated, set its flag to 0 so the
 * rest of BSP can keep building without referencing a missing HAL handle.
 */
#define BOARD_HAS_UART1    1U
#define BOARD_HAS_UART2    1U
#define BOARD_HAS_SPI1     0U
#define BOARD_HAS_SPI2     0U
#define BOARD_HAS_I2C1     0U
#define BOARD_HAS_I2C2     0U
#define BOARD_HAS_CAN1     1U
#define BOARD_HAS_CAN2     0U

#define BOARD_HAS_ANY_SPI  ((BOARD_HAS_SPI1 != 0U) || (BOARD_HAS_SPI2 != 0U))
#define BOARD_HAS_ANY_I2C  ((BOARD_HAS_I2C1 != 0U) || (BOARD_HAS_I2C2 != 0U))
#define BOARD_HAS_ANY_CAN  ((BOARD_HAS_CAN1 != 0U) || (BOARD_HAS_CAN2 != 0U))

#define BOARD_UART_INSTANCE_MAX    2U
#define BOARD_SPI_INSTANCE_MAX     2U
#define BOARD_I2C_INSTANCE_MAX     2U
#define BOARD_CAN_INSTANCE_MAX     2U

UART_HandleTypeDef *board_devices_get_uart(uint8_t instance);

#if BOARD_HAS_ANY_SPI
SPI_HandleTypeDef *board_devices_get_spi(uint8_t instance);
#endif

#if BOARD_HAS_ANY_I2C
I2C_HandleTypeDef *board_devices_get_i2c(uint8_t instance);
#endif

#if BOARD_HAS_ANY_CAN
CAN_HandleTypeDef *board_devices_get_can(uint8_t instance);
#endif

uint8_t board_devices_uart_instance_from_handle(const UART_HandleTypeDef *uart);

#if BOARD_HAS_ANY_SPI
uint8_t board_devices_spi_instance_from_handle(const SPI_HandleTypeDef *spi);
#endif

#if BOARD_HAS_ANY_I2C
uint8_t board_devices_i2c_instance_from_handle(const I2C_HandleTypeDef *i2c);
#endif

#if BOARD_HAS_ANY_CAN
uint8_t board_devices_can_instance_from_handle(const CAN_HandleTypeDef *can);
#endif

#endif /* BOARD_DEVICES_H */
