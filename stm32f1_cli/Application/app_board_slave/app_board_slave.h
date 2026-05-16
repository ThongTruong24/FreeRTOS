#ifndef APP_BOARD_SLAVE_H
#define APP_BOARD_SLAVE_H

#include <stdint.h>

#include "message.h"

void app_board_slave_init(void);
void app_board_slave_rx_byte_from_isr(uint8_t instance, uint8_t byte);
void app_board_slave_handle_message(const app_msg_t *msg);
void app_board_slave_task_entry(void *argument);

#endif /* APP_BOARD_SLAVE_H */
