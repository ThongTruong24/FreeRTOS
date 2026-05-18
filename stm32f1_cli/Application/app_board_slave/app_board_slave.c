#include "app_board_slave.h"

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

#include "app_debug.h"
#include "app_led.h"
#include "app_task.h"
#include "board_service.h"
#include "protocol.h"

#define APP_BOARD_SLAVE_RX_BUFFER_SIZE          128U
#define APP_BOARD_SLAVE_WRITE_TIMEOUT_MS        100U

static StreamBufferHandle_t s_slave_link_rx_stream;
static uint8_t s_slave_link_rx_overflow;
static protocol_parser_t s_slave_link_parser;

static uint8_t app_board_slave_is_link_active(void);
static uint8_t app_board_slave_send_frame(app_msg_id_t id,
                                          const uint8_t *payload,
                                          uint8_t len);
static uint8_t app_board_slave_send_led_control(const app_led_control_msg_t *control);
static uint8_t app_board_slave_send_led_status_request(void);
static uint8_t app_board_slave_send_led_status(void);
static void app_board_slave_encode_led_payload(const app_led_control_msg_t *control,
                                               uint8_t *payload);
static void app_board_slave_encode_led_status_payload(const app_led_status_msg_t *status,
                                                      uint8_t *payload);
static void app_board_slave_decode_frame(const protocol_frame_t *frame);
static void app_board_slave_decode_led_control(const protocol_frame_t *frame);
static void app_board_slave_decode_led_status(const protocol_frame_t *frame);

void app_board_slave_init(void)
{
    s_slave_link_rx_overflow = 0U;
    s_slave_link_rx_stream =
        xStreamBufferCreate(APP_BOARD_SLAVE_RX_BUFFER_SIZE, 1U);
    configASSERT(s_slave_link_rx_stream != NULL);
    protocol_parser_init(&s_slave_link_parser);
}

void app_board_slave_rx_byte(uint8_t byte)
{
    if (xStreamBufferSend(s_slave_link_rx_stream,
                          &byte,
                          sizeof(byte),
                          0U) != sizeof(byte))
    {
        s_slave_link_rx_overflow = 1U;
    }
}

void app_board_slave_rx_byte_from_isr(uint8_t byte)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (xStreamBufferSendFromISR(s_slave_link_rx_stream,
                                 &byte,
                                 sizeof(byte),
                                 &higher_priority_task_woken) != sizeof(byte))
    {
        s_slave_link_rx_overflow = 1U;
    }

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void app_board_slave_handle_message(const app_msg_t *msg)
{
    configASSERT(msg != NULL);

    switch (msg->id)
    {
        case APP_MSG_BOARD_SLAVE_LED_CONTROL:
            if (app_board_slave_send_led_control(&msg->payload.led_control) != 0U)
            {
                app_debug_print("ok\r\n");
            }
            else
            {
                app_debug_print("err: board slave tx failed\r\n");
            }
            break;

        case APP_MSG_BOARD_SLAVE_LED_REQUEST:
            if (app_board_slave_send_led_status_request() != 0U)
            {
                app_debug_print("ok\r\n");
            }
            else
            {
                app_debug_print("err: board slave request failed\r\n");
            }
            break;

        default:
            break;
    }
}

void app_board_slave_task_entry(void *argument)
{
    uint8_t byte;
    protocol_frame_t frame;

    (void)argument;

    for (;;)
    {
        (void)board_service_slave_link_poll_rx(APP_BOARD_SLAVE_WRITE_TIMEOUT_MS);

        if (app_board_slave_is_link_active() == 0U)
        {
            protocol_parser_init(&s_slave_link_parser);
        }

        while (xStreamBufferReceive(s_slave_link_rx_stream,
                                    &byte,
                                    sizeof(byte),
                                    0U) == sizeof(byte))
        {
            if ((app_board_slave_is_link_active() != 0U) &&
                (protocol_parse_byte(&s_slave_link_parser, byte, &frame) != 0U))
            {
                app_board_slave_decode_frame(&frame);
            }
        }

        if (s_slave_link_rx_overflow != 0U)
        {
            s_slave_link_rx_overflow = 0U;
        }

        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}

static uint8_t app_board_slave_is_link_active(void)
{
    return board_service_slave_link_is_active();
}

static uint8_t app_board_slave_send_led_control(const app_led_control_msg_t *control)
{
    uint8_t payload[APP_LED_CONTROL_MSG_SIZE];

    if ((control == NULL) || (app_board_slave_is_link_active() == 0U))
    {
        return 0U;
    }

    app_board_slave_encode_led_payload(control, payload);
    return app_board_slave_send_frame(APP_MSG_LED_CONTROL,
                                      payload,
                                      sizeof(payload));
}

static uint8_t app_board_slave_send_led_status_request(void)
{
    return app_board_slave_send_frame(APP_MSG_LED_STATUS_REQUEST, NULL, 0U);
}

static uint8_t app_board_slave_send_led_status(void)
{
    app_led_status_msg_t status;
    uint8_t payload[APP_LED_STATUS_MSG_SIZE];

    if (app_board_slave_is_link_active() == 0U)
    {
        return 0U;
    }

    app_led_get_status(&status);
    app_board_slave_encode_led_status_payload(&status, payload);

    return app_board_slave_send_frame(APP_MSG_LED_STATUS, payload, sizeof(payload));
}

static uint8_t app_board_slave_send_frame(app_msg_id_t id,
                                          const uint8_t *payload,
                                          uint8_t len)
{
    uint8_t encoded[PROTOCOL_FRAME_MAX_LEN];
    uint8_t encoded_len;

    if (app_board_slave_is_link_active() == 0U)
    {
        return 0U;
    }

    encoded_len = protocol_encode_frame(id,
                                        payload,
                                        len,
                                        encoded,
                                        sizeof(encoded));

    if (encoded_len == 0U)
    {
        return 0U;
    }

    return board_service_slave_link_write(encoded,
                                          encoded_len,
                                          APP_BOARD_SLAVE_WRITE_TIMEOUT_MS);
}

static void app_board_slave_encode_led_payload(const app_led_control_msg_t *control,
                                               uint8_t *payload)
{
    payload[0] = control->led_id;
    payload[1] = control->command;
    payload[2] = (uint8_t)(control->period_ms & 0xFFU);
    payload[3] = (uint8_t)(control->period_ms >> 8);
    payload[4] = (uint8_t)(control->hz & 0xFFU);
    payload[5] = (uint8_t)(control->hz >> 8);
    payload[6] = (uint8_t)(control->count & 0xFFU);
    payload[7] = (uint8_t)(control->count >> 8);
}

static void app_board_slave_encode_led_status_payload(const app_led_status_msg_t *status,
                                                      uint8_t *payload)
{
    payload[0] = status->led_id;
    payload[1] = status->state;
    payload[2] = status->blinking;
    payload[3] = status->reserved;
    payload[4] = (uint8_t)(status->hz & 0xFFU);
    payload[5] = (uint8_t)(status->hz >> 8);
    payload[6] = (uint8_t)(status->remaining_count & 0xFFU);
    payload[7] = (uint8_t)(status->remaining_count >> 8);
}

static void app_board_slave_decode_frame(const protocol_frame_t *frame)
{
    if (frame == NULL)
    {
        return;
    }

    switch (frame->message_id)
    {
        case APP_MSG_LED_CONTROL:
            app_board_slave_decode_led_control(frame);
            break;

        case APP_MSG_LED_STATUS_REQUEST:
            (void)app_board_slave_send_led_status();
            break;

        case APP_MSG_LED_STATUS:
            app_board_slave_decode_led_status(frame);
            break;

        default:
            break;
    }
}

static void app_board_slave_decode_led_control(const protocol_frame_t *frame)
{
    app_msg_t msg = {0};

    if ((frame == NULL) || (frame->len < APP_LED_CONTROL_MSG_SIZE))
    {
        return;
    }

    msg.id = APP_MSG_LED_CONTROL;
    msg.len = sizeof(msg.payload.led_control);
    msg.payload.led_control.led_id = frame->payload[0];
    msg.payload.led_control.command = frame->payload[1];
    msg.payload.led_control.period_ms =
        (uint16_t)frame->payload[2] | ((uint16_t)frame->payload[3] << 8);
    msg.payload.led_control.hz =
        (uint16_t)frame->payload[4] | ((uint16_t)frame->payload[5] << 8);
    msg.payload.led_control.count =
        (uint16_t)frame->payload[6] | ((uint16_t)frame->payload[7] << 8);

    (void)app_task_post(&msg, 0U);
}

static void app_board_slave_decode_led_status(const protocol_frame_t *frame)
{
    app_msg_t msg = {0};

    if ((frame == NULL) || (frame->len < APP_LED_STATUS_MSG_SIZE))
    {
        return;
    }

    msg.id = APP_MSG_LED_STATUS;
    msg.len = sizeof(msg.payload.led_status);
    msg.payload.led_status.led_id = frame->payload[0];
    msg.payload.led_status.state = frame->payload[1];
    msg.payload.led_status.blinking = frame->payload[2];
    msg.payload.led_status.reserved = frame->payload[3];
    msg.payload.led_status.hz =
        (uint16_t)frame->payload[4] | ((uint16_t)frame->payload[5] << 8);
    msg.payload.led_status.remaining_count =
        (uint16_t)frame->payload[6] | ((uint16_t)frame->payload[7] << 8);

    (void)app_task_post(&msg, 0U);
}
