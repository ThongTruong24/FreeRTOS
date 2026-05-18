#include "app_led.h"

#include "FreeRTOS.h"
#include "task.h"

#include "app_task.h"
#include "board_service.h"
#include "param.h"

#define APP_LED_DEFAULT_ID    0U

static uint8_t s_blink_enabled;
static uint8_t s_counted_blink;
static uint8_t s_led_state;
static uint16_t s_blink_hz;
static uint16_t s_blink_period_ms;
static uint16_t s_remaining_toggles;
static uint32_t s_last_toggle_ms;

static uint16_t hz_to_toggle_period_ms(uint16_t hz);
static void app_led_stop_blink(void);
static void app_led_apply_control(const app_led_control_msg_t *control, uint32_t now_ms);
static void app_led_handle_tick(uint32_t now_ms);

void app_led_init(void)
{
    board_service_status_led_init();
    s_blink_enabled = 0U;
    s_counted_blink = 0U;
    s_led_state = LED_STATE_OFF;
    s_blink_hz = 0U;
    s_blink_period_ms = hz_to_toggle_period_ms((uint16_t)param_get(PARAM_LED_DEFAULT_HZ));
    s_remaining_toggles = 0U;
    s_last_toggle_ms = 0U;
    board_service_status_led_off(APP_LED_DEFAULT_ID);
}

void app_led_on(uint8_t led_id)
{
    board_service_status_led_on(led_id);
}

void app_led_off(uint8_t led_id)
{
    board_service_status_led_off(led_id);
}

void app_led_toggle(uint8_t led_id)
{
    board_service_status_led_toggle(led_id);
}

void app_led_get_status(app_led_status_msg_t *status)
{
    if (status == NULL)
    {
        return;
    }

    status->led_id = APP_LED_DEFAULT_ID;
    status->state = s_led_state;
    status->blinking = s_blink_enabled;
    status->reserved = 0U;
    status->hz = s_blink_hz;
    status->remaining_count = (uint16_t)((s_remaining_toggles + 1U) / 2U);
}

void app_led_handle_message(const app_msg_t *msg)
{
    configASSERT(msg != NULL);

    switch (msg->id)
    {
        case APP_MSG_LED_CONTROL:
            app_led_apply_control(&msg->payload.led_control, msg->timestamp_ms);
            break;

        case APP_MSG_LED_TICK:
            app_led_handle_tick(msg->timestamp_ms);
            break;

        default:
            break;
    }
}

void app_led_task_entry(void *argument)
{
    TickType_t last_wake_time;
    app_msg_t msg = {0};

    (void)argument;

    last_wake_time = xTaskGetTickCount();
    msg.id = APP_MSG_LED_TICK;

    for (;;)
    {
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(10U));
        (void)app_task_post(&msg, 0U);
    }
}

static uint16_t hz_to_toggle_period_ms(uint16_t hz)
{
    uint32_t period_ms;

    if (hz == 0U)
    {
        return 500U;
    }

    period_ms = 1000UL / ((uint32_t)hz * 2UL);

    if (period_ms == 0UL)
    {
        period_ms = 1UL;
    }

    if (period_ms > 65535UL)
    {
        period_ms = 65535UL;
    }

    return (uint16_t)period_ms;
}

static void app_led_apply_control(const app_led_control_msg_t *control, uint32_t now_ms)
{
    if (control == NULL)
    {
        return;
    }

    switch (control->command)
    {
        case LED_CMD_ON:
            app_led_stop_blink();
            s_led_state = LED_STATE_ON;
            app_led_on(control->led_id);
            break;

        case LED_CMD_OFF:
            app_led_stop_blink();
            s_led_state = LED_STATE_OFF;
            app_led_off(control->led_id);
            break;

        case LED_CMD_TOGGLE:
            app_led_stop_blink();
            s_led_state = (s_led_state == LED_STATE_ON) ? LED_STATE_OFF : LED_STATE_ON;
            app_led_toggle(control->led_id);
            break;

        case LED_CMD_BLINK:
            s_blink_enabled = 1U;
            s_counted_blink = 0U;
            s_blink_hz = 0U;
            s_blink_period_ms = (control->period_ms == 0U) ? 500U : control->period_ms;
            s_last_toggle_ms = now_ms;
            break;

        case LED_CMD_BLINK_HZ_COUNT:
            s_blink_enabled = 1U;
            s_counted_blink = (control->count == 0U) ? 0U : 1U;
            s_led_state = LED_STATE_OFF;
            s_blink_hz = control->hz;
            s_blink_period_ms = hz_to_toggle_period_ms(control->hz);
            s_remaining_toggles =
                (control->count > 32767U) ? 65535U : (uint16_t)(control->count * 2U);
            s_last_toggle_ms = now_ms;
            app_led_off(control->led_id);
            break;

        default:
            break;
    }
}

static void app_led_handle_tick(uint32_t now_ms)
{
    if ((s_blink_enabled == 0U) ||
        ((now_ms - s_last_toggle_ms) < s_blink_period_ms))
    {
        return;
    }

    s_last_toggle_ms = now_ms;
    s_led_state = (s_led_state == LED_STATE_ON) ? LED_STATE_OFF : LED_STATE_ON;
    app_led_toggle(APP_LED_DEFAULT_ID);

    if (s_counted_blink != 0U)
    {
        if (s_remaining_toggles > 0U)
        {
            s_remaining_toggles--;
        }

        if (s_remaining_toggles == 0U)
        {
            app_led_stop_blink();
            s_led_state = LED_STATE_OFF;
            app_led_off(APP_LED_DEFAULT_ID);
        }
    }
}

static void app_led_stop_blink(void)
{
    s_blink_enabled = 0U;
    s_counted_blink = 0U;
    s_blink_hz = 0U;
    s_remaining_toggles = 0U;
}
