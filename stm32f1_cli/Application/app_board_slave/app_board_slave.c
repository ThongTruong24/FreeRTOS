#include "app_board_slave.h"

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

#include "app_debug.h"
#include "app_task.h"
#include "param.h"
#include "protocol.h"
#include "uart_port.h"

#define APP_BOARD_SLAVE_UART1_RX_BUFFER_SIZE    128U

static StreamBufferHandle_t s_uart1_rx_stream;
static uint8_t s_uart1_rx_overflow;
static protocol_parser_t s_uart1_parser;

static uint8_t app_board_slave_uart1_is_slave_link(void);
static uint8_t app_board_slave_send_led_control(const app_led_control_msg_t *control);
static void app_board_slave_encode_led_payload(const app_led_control_msg_t *control,
                                               uint8_t *payload);
static void app_board_slave_decode_frame(const protocol_frame_t *frame);
static void app_board_slave_decode_led_control(const protocol_frame_t *frame);

void app_board_slave_init(void)
{
    s_uart1_rx_overflow = 0U;
    s_uart1_rx_stream =
        xStreamBufferCreate(APP_BOARD_SLAVE_UART1_RX_BUFFER_SIZE, 1U);
    configASSERT(s_uart1_rx_stream != NULL);
    protocol_parser_init(&s_uart1_parser);
}

void app_board_slave_rx_byte_from_isr(uint8_t instance, uint8_t byte)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (instance != 1U)
    {
        return;
    }

    if (xStreamBufferSendFromISR(s_uart1_rx_stream,
                                 &byte,
                                 sizeof(byte),
                                 &higher_priority_task_woken) != sizeof(byte))
    {
        s_uart1_rx_overflow = 1U;
    }

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void app_board_slave_handle_message(const app_msg_t *msg)
{
    configASSERT(msg != NULL);

    if (msg->id == APP_MSG_BOARD_SLAVE_LED_CONTROL)
    {
        if (app_board_slave_send_led_control(&msg->payload.led_control) != 0U)
        {
            app_debug_print("ok\r\n");
        }
        else
        {
            app_debug_print("err: board slave tx failed\r\n");
        }

        return;
    }

    return;
}

void app_board_slave_task_entry(void *argument)
{
    uint8_t byte;
    protocol_frame_t frame;

    (void)argument;

    for (;;)
    {
        if (app_board_slave_uart1_is_slave_link() == 0U)
        {
            protocol_parser_init(&s_uart1_parser);
        }

        while (xStreamBufferReceive(s_uart1_rx_stream,
                                    &byte,
                                    sizeof(byte),
                                    0U) == sizeof(byte))
        {
            if ((app_board_slave_uart1_is_slave_link() != 0U) &&
                (protocol_parse_byte(&s_uart1_parser, byte, &frame) != 0U))
            {
                app_board_slave_decode_frame(&frame);
            }
        }

        if (s_uart1_rx_overflow != 0U)
        {
            s_uart1_rx_overflow = 0U;
        }

        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}

static uint8_t app_board_slave_uart1_is_slave_link(void)
{
    return (uint8_t)((param_get(PARAM_UART1_EN) != 0) &&
                     (param_get(PARAM_UART1_PROTOCOL) ==
                      UART1_PROTOCOL_SLAVE_CONTROL));
}

static uint8_t app_board_slave_send_led_control(const app_led_control_msg_t *control)
{
    uint8_t payload[APP_LED_CONTROL_MSG_SIZE];
    uint8_t encoded[PROTOCOL_FRAME_MAX_LEN];
    uint8_t encoded_len;

    if ((control == NULL) || (app_board_slave_uart1_is_slave_link() == 0U))
    {
        return 0U;
    }

    app_board_slave_encode_led_payload(control, payload);
    encoded_len = protocol_encode_frame(APP_MSG_LED_CONTROL,
                                        payload,
                                        sizeof(payload),
                                        encoded,
                                        sizeof(encoded));

    if (encoded_len == 0U)
    {
        return 0U;
    }

    return uart_port_write(1U, encoded, encoded_len, 100U);
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
