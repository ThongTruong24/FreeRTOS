#ifndef SPI_PORT_H
#define SPI_PORT_H

#include <stdint.h>

#include "board_devices.h"

uint8_t spi_port_is_enabled(uint8_t instance);
uint8_t spi_port_write(uint8_t instance, const uint8_t *data, uint8_t len, uint32_t timeout_ms);
uint8_t spi_port_read(uint8_t instance, uint8_t *data, uint8_t len, uint32_t timeout_ms);
uint8_t spi_port_transfer(uint8_t instance, const uint8_t *tx, uint8_t *rx, uint8_t len,
                         uint32_t timeout_ms);
uint8_t spi_port_is_slave(uint8_t instance);
uint8_t spi_port_start_slave_transfer_it(uint8_t instance, uint8_t *tx, uint8_t *rx,
                                         uint8_t len);
uint8_t spi_port_abort_it(uint8_t instance);
#if BOARD_HAS_ANY_SPI
uint8_t spi_port_instance_from_handle(const SPI_HandleTypeDef *spi);
#endif
uint8_t spi_port_init(uint8_t instance);
uint8_t spi_port_deinit(uint8_t instance);

#endif
