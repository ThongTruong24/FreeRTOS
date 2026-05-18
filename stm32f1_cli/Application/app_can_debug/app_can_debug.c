#include "app_can_debug.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_task.h"
#include "board_service.h"

static void app_can_debug_post_status(uint8_t instance,
                                      app_can_status_event_t event,
                                      uint32_t error_code);

void app_can_debug_init(void)
{
    uint8_t instance = board_service_can_debug_instance();

    if (instance == 0U)
    {
        return;
    }

    if (board_service_can_debug_init() != 0U)
    {
        app_can_debug_post_status(instance, CAN_STATUS_STARTED, 0U);
    }
    else
    {
        if (board_service_can_tx_mailboxes_are_full(instance) != 0U)
        {
            app_can_debug_post_status(instance,
                                      CAN_STATUS_TX_MAILBOX_FULL,
                                      0U);
        }
        else
        {
            app_can_debug_post_status(instance,
                                      CAN_STATUS_ERROR,
                                      board_service_can_take_error(instance));
        }
    }
}

void app_can_debug_handle_message(const app_msg_t *msg)
{
    uint8_t instance;

    configASSERT(msg != NULL);

    if (msg->id != APP_MSG_CAN_DEBUG_TX)
    {
        return;
    }

    instance = board_service_can_debug_instance();

    if (board_service_can_tx_mailboxes_are_full(instance) != 0U)
    {
        app_can_debug_post_status(instance,
                                  CAN_STATUS_TX_MAILBOX_FULL,
                                  0U);
        return;
    }

    if (board_service_can_debug_write(msg->payload.can_frame.std_id,
                                      msg->payload.can_frame.data,
                                      msg->payload.can_frame.len) != 0U)
    {
        app_can_debug_post_status(instance, CAN_STATUS_TX_QUEUED, 0U);
    }
    else
    {
        if (board_service_can_tx_mailboxes_are_full(instance) != 0U)
        {
            app_can_debug_post_status(instance,
                                      CAN_STATUS_TX_MAILBOX_FULL,
                                      0U);
        }
        else
        {
            app_can_debug_post_status(instance,
                                      CAN_STATUS_ERROR,
                                      board_service_can_take_error(instance));
        }
    }
}

static void app_can_debug_post_status(uint8_t instance,
                                      app_can_status_event_t event,
                                      uint32_t error_code)
{
    app_msg_t msg = {0};

    msg.id = APP_MSG_CAN_STATUS;
    msg.len = sizeof(msg.payload.can_status);
    msg.payload.can_status.instance = instance;
    msg.payload.can_status.event = (uint8_t)event;
    msg.payload.can_status.error_code = error_code;
    (void)app_task_post(&msg, 0U);
}
