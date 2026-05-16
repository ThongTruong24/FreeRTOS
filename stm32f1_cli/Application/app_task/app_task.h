#ifndef APP_TASK_H
#define APP_TASK_H

#include "FreeRTOS.h"

#include "message.h"

/* Create application-owned IPC objects. */
void app_task_init(void);

/* Create all application tasks from one central place. */
void app_task_start(void);

/* Post a message from task context. */
BaseType_t app_task_post(const app_msg_t *msg, TickType_t timeout_ticks);

/* Post a message from ISR context. */
BaseType_t app_task_post_from_isr(const app_msg_t *msg,
                                 BaseType_t *higher_priority_task_woken);

#endif /* APP_TASK_H */
