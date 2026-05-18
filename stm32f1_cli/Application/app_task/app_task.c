#include "app_task.h"

#include "queue.h"
#include "task.h"

#include "app_board_slave.h"
#include "app_can_debug.h"
#include "app_debug.h"
#include "app_gps.h"
#include "app_led.h"

#define APP_QUEUE_LENGTH                   16U

#define APP_MAIN_TASK_STACK_DEPTH          configMINIMAL_STACK_SIZE
#define APP_LED_TASK_STACK_DEPTH           configMINIMAL_STACK_SIZE
#define APP_DEBUG_TASK_STACK_DEPTH         (configMINIMAL_STACK_SIZE * 2U)
#define APP_GPS_TASK_STACK_DEPTH           configMINIMAL_STACK_SIZE
#define APP_BOARD_SLAVE_TASK_STACK_DEPTH   configMINIMAL_STACK_SIZE

#define APP_MAIN_TASK_PRIORITY             3U
#define APP_BOARD_SLAVE_TASK_PRIORITY      2U
#define APP_GPS_TASK_PRIORITY              2U
#define APP_LED_TASK_PRIORITY              1U
#define APP_DEBUG_TASK_PRIORITY            1U

static QueueHandle_t s_app_queue = NULL;

static void app_main_task(void *argument);
static uint32_t app_task_ticks_to_ms(TickType_t ticks);

void app_task_init(void)
{
    configASSERT(s_app_queue == NULL);

    s_app_queue = xQueueCreate(APP_QUEUE_LENGTH, sizeof(app_msg_t));
    configASSERT(s_app_queue != NULL);
}

void app_task_start(void)
{
    BaseType_t status;
    app_msg_t start_msg = {0};

    configASSERT(s_app_queue != NULL);

    status = xTaskCreate(app_main_task,
                         "app_main",
                         APP_MAIN_TASK_STACK_DEPTH,
                         NULL,
                         APP_MAIN_TASK_PRIORITY,
                         NULL);
    configASSERT(status == pdPASS);

    status = xTaskCreate(app_led_task_entry,
                         "app_led",
                         APP_LED_TASK_STACK_DEPTH,
                         NULL,
                         APP_LED_TASK_PRIORITY,
                         NULL);
    configASSERT(status == pdPASS);

    status = xTaskCreate(app_debug_task_entry,
                         "app_debug",
                         APP_DEBUG_TASK_STACK_DEPTH,
                         NULL,
                         APP_DEBUG_TASK_PRIORITY,
                         NULL);
    configASSERT(status == pdPASS);

    status = xTaskCreate(app_gps_task_entry,
                         "app_gps",
                         APP_GPS_TASK_STACK_DEPTH,
                         NULL,
                         APP_GPS_TASK_PRIORITY,
                         NULL);
    configASSERT(status == pdPASS);

    status = xTaskCreate(app_board_slave_task_entry,
                         "board_slave",
                         APP_BOARD_SLAVE_TASK_STACK_DEPTH,
                         NULL,
                         APP_BOARD_SLAVE_TASK_PRIORITY,
                         NULL);
    configASSERT(status == pdPASS);

    start_msg.id = APP_MSG_SYSTEM_START;
    status = app_task_post(&start_msg, 0U);
    configASSERT(status == pdPASS);
}

BaseType_t app_task_post(const app_msg_t *msg, TickType_t timeout_ticks)
{
    app_msg_t queued_msg;

    configASSERT(msg != NULL);
    configASSERT(s_app_queue != NULL);

    queued_msg = *msg;
    queued_msg.timestamp_ms = app_task_ticks_to_ms(xTaskGetTickCount());

    return xQueueSend(s_app_queue, &queued_msg, timeout_ticks);
}

BaseType_t app_task_post_from_isr(const app_msg_t *msg,
                                 BaseType_t *higher_priority_task_woken)
{
    app_msg_t queued_msg;

    configASSERT(msg != NULL);
    configASSERT(s_app_queue != NULL);

    queued_msg = *msg;
    queued_msg.timestamp_ms = app_task_ticks_to_ms(xTaskGetTickCountFromISR());

    return xQueueSendFromISR(s_app_queue,
                             &queued_msg,
                             higher_priority_task_woken);
}

static void app_main_task(void *argument)
{
    app_msg_t msg;

    (void)argument;

    for (;;)
    {
        if (xQueueReceive(s_app_queue, &msg, portMAX_DELAY) == pdPASS)
        {
            switch (msg.id)
            {
                case APP_MSG_LED_CONTROL:
                case APP_MSG_LED_TICK:
                    app_led_handle_message(&msg);
                    break;

                case APP_MSG_BOARD_SLAVE_LED_CONTROL:
                case APP_MSG_BOARD_SLAVE_LED_REQUEST:
                    app_board_slave_handle_message(&msg);
                    break;

                case APP_MSG_CAN_DEBUG_TX:
                    app_can_debug_handle_message(&msg);
                    break;

                case APP_MSG_LED_STATUS:
                case APP_MSG_CAN_STATUS:
                    app_debug_handle_message(&msg);
                    break;

                case APP_MSG_SYSTEM_START:
                    app_debug_print("System started\r\n");
                    break;

                case APP_MSG_SYSTEM_ERROR:
                    app_debug_print("System error\r\n");
                    break;

                case APP_MSG_NONE:
                case APP_MSG_MAX:
                default:
                    break;
            }
        }
    }
}

static uint32_t app_task_ticks_to_ms(TickType_t ticks)
{
    return (uint32_t)pdTICKS_TO_MS(ticks);
}
