#ifndef SLAVE_LINK_PORT_H
#define SLAVE_LINK_PORT_H

#include <stdint.h>

uint8_t slave_link_port_stage_tx_frame(const uint8_t *data, uint8_t len);

uint8_t slave_link_port_uart_write(uint8_t instance,
                                   const uint8_t *data,
                                   uint8_t len,
                                   uint32_t timeout_ms);
void slave_link_port_uart_rx_byte_from_isr(uint8_t byte);

uint8_t slave_link_port_spi_master_write(uint8_t instance,
                                         const uint8_t *data,
                                         uint8_t len,
                                         uint32_t timeout_ms);
uint8_t slave_link_port_spi_master_poll_rx(uint8_t instance, uint32_t timeout_ms);
uint8_t slave_link_port_spi_slave_start(uint8_t instance);
void slave_link_port_spi_slave_transfer_complete_from_isr(uint8_t instance);
void slave_link_port_spi_slave_error_from_isr(uint8_t instance);

uint8_t slave_link_port_i2c_master_write(uint8_t instance,
                                         uint8_t addr7,
                                         const uint8_t *data,
                                         uint8_t len,
                                         uint32_t timeout_ms);
uint8_t slave_link_port_i2c_master_poll_rx(uint8_t instance,
                                           uint8_t addr7,
                                           uint32_t timeout_ms);
uint8_t slave_link_port_i2c_slave_start(uint8_t instance);
void slave_link_port_i2c_slave_addr_from_isr(uint8_t instance, uint8_t transfer_direction);
void slave_link_port_i2c_slave_rx_complete_from_isr(uint8_t instance);
void slave_link_port_i2c_slave_tx_complete_from_isr(uint8_t instance);
void slave_link_port_i2c_slave_error_from_isr(uint8_t instance);

#endif /* SLAVE_LINK_PORT_H */
