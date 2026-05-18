#ifndef BOARD_SERVICE_H
#define BOARD_SERVICE_H

#include <stdint.h>

typedef enum
{
    BOARD_SERVICE_CAN_ERROR_NONE = 0U,
    BOARD_SERVICE_CAN_ERROR_WARNING = (1UL << 0),
    BOARD_SERVICE_CAN_ERROR_PASSIVE = (1UL << 1),
    BOARD_SERVICE_CAN_ERROR_BUS_OFF = (1UL << 2),
    BOARD_SERVICE_CAN_ERROR_STUFF = (1UL << 3),
    BOARD_SERVICE_CAN_ERROR_FORM = (1UL << 4),
    BOARD_SERVICE_CAN_ERROR_ACK = (1UL << 5),
    BOARD_SERVICE_CAN_ERROR_BIT_RECESSIVE = (1UL << 6),
    BOARD_SERVICE_CAN_ERROR_BIT_DOMINANT = (1UL << 7),
    BOARD_SERVICE_CAN_ERROR_CRC = (1UL << 8),
    BOARD_SERVICE_CAN_ERROR_RX_OVERRUN0 = (1UL << 9),
    BOARD_SERVICE_CAN_ERROR_RX_OVERRUN1 = (1UL << 10),
    BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST0 = (1UL << 11),
    BOARD_SERVICE_CAN_ERROR_TX0 = (1UL << 12),
    BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST1 = (1UL << 13),
    BOARD_SERVICE_CAN_ERROR_TX1 = (1UL << 14),
    BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST2 = (1UL << 15),
    BOARD_SERVICE_CAN_ERROR_TX2 = (1UL << 16),
    BOARD_SERVICE_CAN_ERROR_TIMEOUT = (1UL << 17),
    BOARD_SERVICE_CAN_ERROR_NOT_INITIALIZED = (1UL << 18),
    BOARD_SERVICE_CAN_ERROR_NOT_READY = (1UL << 19),
    BOARD_SERVICE_CAN_ERROR_NOT_STARTED = (1UL << 20),
    BOARD_SERVICE_CAN_ERROR_PARAM = (1UL << 21),
    BOARD_SERVICE_CAN_ERROR_INTERNAL = (1UL << 22)
} board_service_can_error_t;

void board_service_init(void);

void board_service_console_init(void);
void board_service_console_write(const uint8_t *data, uint16_t len);
uint8_t board_service_console_uses_uart(uint8_t instance);
uint8_t board_service_gps_uses_uart(uint8_t instance);

uint8_t board_service_can_debug_init(void);
uint8_t board_service_can_debug_write(uint16_t std_id,
                                      const uint8_t *data,
                                      uint8_t len);
uint8_t board_service_can_debug_instance(void);
uint8_t board_service_can_tx_mailboxes_are_full(uint8_t instance);
uint8_t board_service_can_uses_debug(uint8_t instance);
uint32_t board_service_can_take_error(uint8_t instance);

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
