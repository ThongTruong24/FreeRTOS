#ifndef APP_CAN_DEBUG_H
#define APP_CAN_DEBUG_H

#include "message.h"

void app_can_debug_init(void);
void app_can_debug_handle_message(const app_msg_t *msg);

#endif /* APP_CAN_DEBUG_H */
