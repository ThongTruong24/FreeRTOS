#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#include "message.h"

void app_debug_init(void);
void app_debug_print(const char *text);
void app_debug_rx_byte_from_isr(uint8_t byte);
void app_debug_handle_message(const app_msg_t *msg);
void app_debug_task_entry(void *argument);

#endif /* APP_DEBUG_H */
