#ifndef BOARD_SERVICE_H
#define BOARD_SERVICE_H

#include <stdint.h>

void board_service_init(void);

void board_service_console_init(void);
void board_service_console_write(const uint8_t *data, uint16_t len);
uint8_t board_service_console_uses_uart(uint8_t instance);
uint8_t board_service_gps_uses_uart(uint8_t instance);

void board_service_status_led_init(void);
void board_service_status_led_on(uint8_t led_id);
void board_service_status_led_off(uint8_t led_id);
void board_service_status_led_toggle(uint8_t led_id);
void board_service_status_led_toggle_default(void);

uint8_t board_service_slave_link_is_active(void);
uint8_t board_service_slave_link_write(const uint8_t *data,
                                       uint8_t len,
                                       uint32_t timeout_ms);
uint8_t board_service_slave_link_poll_rx(uint32_t timeout_ms);
uint8_t board_service_slave_link_start_rx(void);
uint8_t board_service_slave_link_uses_uart(uint8_t instance);
uint8_t board_service_slave_link_uses_spi(uint8_t instance);
uint8_t board_service_slave_link_uses_i2c(uint8_t instance);

#endif /* BOARD_SERVICE_H */
