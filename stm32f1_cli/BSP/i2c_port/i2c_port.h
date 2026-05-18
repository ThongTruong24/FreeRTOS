#ifndef I2C_PORT_H
#define I2C_PORT_H

#include <stdint.h>

#include "board_devices.h"

uint8_t i2c_port_is_enabled(uint8_t instance);
uint8_t i2c_port_master_write(uint8_t instance, uint8_t addr7, const uint8_t *data, uint8_t len,
                             uint32_t timeout_ms);
uint8_t i2c_port_master_read(uint8_t instance, uint8_t addr7, uint8_t *data, uint8_t len,
                            uint32_t timeout_ms);
uint8_t i2c_port_slave_write(uint8_t instance, const uint8_t *data, uint8_t len,
                            uint32_t timeout_ms);
uint8_t i2c_port_start_slave_receive_it(uint8_t instance, uint8_t *data, uint8_t len);
uint8_t i2c_port_start_slave_transmit_it(uint8_t instance, uint8_t *data, uint8_t len);
uint8_t i2c_port_start_slave_seq_receive_it(uint8_t instance, uint8_t *data, uint8_t len);
uint8_t i2c_port_start_slave_seq_transmit_it(uint8_t instance, uint8_t *data, uint8_t len);
uint8_t i2c_port_enable_listen_it(uint8_t instance);
#if BOARD_HAS_ANY_I2C
uint8_t i2c_port_instance_from_handle(const I2C_HandleTypeDef *i2c);
#endif
uint8_t i2c_port_init(uint8_t instance);
uint8_t i2c_port_deinit(uint8_t instance);

#endif
