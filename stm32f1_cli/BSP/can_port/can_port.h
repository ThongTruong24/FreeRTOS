#ifndef CAN_PORT_H
#define CAN_PORT_H

#include <stdint.h>

#include "board_devices.h"

uint8_t can_port_is_enabled(uint8_t instance);
uint8_t can_port_start(uint8_t instance);
uint8_t can_port_stop(uint8_t instance);
uint8_t can_port_write_std(uint8_t instance,
                           uint16_t std_id,
                           const uint8_t *data,
                           uint8_t len);
uint8_t can_port_tx_mailboxes_are_full(uint8_t instance);
uint32_t can_port_take_error(uint8_t instance);

#if BOARD_HAS_ANY_CAN
uint8_t can_port_instance_from_handle(const CAN_HandleTypeDef *can);
#endif

#endif /* CAN_PORT_H */
