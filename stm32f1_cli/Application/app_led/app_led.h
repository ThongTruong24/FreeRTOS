#ifndef APP_LED_H
#define APP_LED_H

#include <stdint.h>

#include "message.h"

void app_led_init(void);
void app_led_on(uint8_t led_id);
void app_led_off(uint8_t led_id);
void app_led_toggle(uint8_t led_id);
void app_led_handle_message(const app_msg_t *msg);
void app_led_task_entry(void *argument);

#endif /* APP_LED_H */
