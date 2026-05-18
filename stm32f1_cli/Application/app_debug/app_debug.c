#include "app_debug.h"

#include <stdint.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

#include "app_gps.h"
#include "app_task.h"
#include "board_service.h"
#include "param.h"

#define APP_DEBUG_RX_BUFFER_SIZE       128U
#define APP_DEBUG_COMMAND_MAX_LEN      80U
#define APP_DEBUG_ARG_MAX_COUNT        8U

static StreamBufferHandle_t s_rx_stream;
static char s_command[APP_DEBUG_COMMAND_MAX_LEN];
static uint8_t s_command_len;
static uint8_t s_rx_overflow;

static void app_debug_process_rx(void);
static void app_debug_execute_command(char *command);
static int app_debug_split_args(char *command, char **argv, int argv_max);
static void app_debug_handle_led_command(int argc, char **argv);
static void app_debug_handle_param_command(int argc, char **argv);
static void app_debug_handle_gps_command(int argc, char **argv);
static void app_debug_handle_can_command(int argc, char **argv);
static void app_debug_print_command_list(void);
static void app_debug_print_led_usage(void);
static void app_debug_print_board_slave_led_usage(void);
static uint8_t app_debug_parse_local_led_command(int argc,
                                                 char **argv,
                                                 app_led_control_msg_t *control);
static uint8_t app_debug_parse_board_slave_led_command(
    int argc,
    char **argv,
    app_led_control_msg_t *control);
static uint8_t app_debug_parse_led_action(const char *token, uint8_t *command);
static uint8_t app_debug_parse_led_blink(int argc,
                                         char **argv,
                                         int start_index,
                                         app_led_control_msg_t *control);
static uint8_t app_debug_parse_led_blink_hz(int argc,
                                            char **argv,
                                            int start_index,
                                            app_led_control_msg_t *control);
static uint8_t app_debug_parse_u16(const char *token, uint16_t *value);
static uint8_t app_debug_parse_u32_limit(const char *token,
                                         uint32_t max_value,
                                         uint32_t *value);
static uint8_t app_debug_ascii_to_nibble(char c, uint8_t *value);
static uint8_t app_debug_streq(const char *a, const char *b);
static uint16_t app_debug_text_len(const char *text);
static void app_debug_post_led_control(app_msg_id_t id,
                                       const app_led_control_msg_t *control);
static void app_debug_post_board_slave_led_request(void);
static void app_debug_post_can_debug_tx(const app_can_frame_msg_t *frame);
static void app_debug_print_param_table(void);
static void app_debug_print_gps_snapshot(void);
static void app_debug_print_text_or_dash(const char *text);
static void app_debug_print_can_error_detail(uint32_t error_code);
static void app_debug_print_can_error_group(uint32_t error_code,
                                            uint32_t group_mask,
                                            const char *label);
static void app_debug_write_i32(int32_t value);
static void app_debug_write_u32(uint32_t value);

void app_debug_init(void)
{
    board_service_console_init();
    s_command_len = 0U;
    s_rx_overflow = 0U;
    s_rx_stream = xStreamBufferCreate(APP_DEBUG_RX_BUFFER_SIZE, 1U);
    configASSERT(s_rx_stream != NULL);
}

void app_debug_print(const char *text)
{
    uint16_t len;

    if (text == NULL)
    {
        return;
    }

    len = app_debug_text_len(text);

    if (len > 0U)
    {
        board_service_console_write((const uint8_t *)text, len);
    }
}

void app_debug_rx_byte_from_isr(uint8_t byte)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    if (xStreamBufferSendFromISR(s_rx_stream,
                                 &byte,
                                 sizeof(byte),
                                 &higher_priority_task_woken) != sizeof(byte))
    {
        s_rx_overflow = 1U;
    }

    portYIELD_FROM_ISR(higher_priority_task_woken);
}

void app_debug_handle_message(const app_msg_t *msg)
{
    configASSERT(msg != NULL);

    switch (msg->id)
    {
        case APP_MSG_LED_STATUS:
            app_debug_print("led board slave status: ");
            app_debug_print((msg->payload.led_status.state == LED_STATE_ON) ?
                            "on" : "off");
            app_debug_print(" blinking=");
            app_debug_write_u32(msg->payload.led_status.blinking);
            app_debug_print(" hz=");
            app_debug_write_u32(msg->payload.led_status.hz);
            app_debug_print(" remaining=");
            app_debug_write_u32(msg->payload.led_status.remaining_count);
            app_debug_print("\r\n");
            break;

        case APP_MSG_CAN_STATUS:
            if (msg->payload.can_status.event == CAN_STATUS_STARTED)
            {
                app_debug_print("can debug ready instance=");
                app_debug_write_u32(msg->payload.can_status.instance);
                app_debug_print("\r\n");
            }
            else if (msg->payload.can_status.event == CAN_STATUS_TX_QUEUED)
            {
                app_debug_print("ok\r\n");
            }
            else if (msg->payload.can_status.event == CAN_STATUS_TX_ABORTED)
            {
                app_debug_print("err: can tx aborted instance=");
                app_debug_write_u32(msg->payload.can_status.instance);
                app_debug_print_can_error_detail(msg->payload.can_status.error_code);
                app_debug_print(" code=");
                app_debug_write_u32(msg->payload.can_status.error_code);
                app_debug_print("\r\n");
            }
            else if (msg->payload.can_status.event == CAN_STATUS_TX_MAILBOX_FULL)
            {
                app_debug_print("err: can tx mailbox full instance=");
                app_debug_write_u32(msg->payload.can_status.instance);
                app_debug_print("\r\n");
            }
            else if (msg->payload.can_status.event == CAN_STATUS_ERROR)
            {
                app_debug_print("err: can instance=");
                app_debug_write_u32(msg->payload.can_status.instance);
                app_debug_print_can_error_detail(msg->payload.can_status.error_code);
                app_debug_print(" code=");
                app_debug_write_u32(msg->payload.can_status.error_code);
                app_debug_print("\r\n");
            }
            break;

        default:
            break;
    }
}

void app_debug_task_entry(void *argument)
{
    (void)argument;

    app_debug_print("\r\nstm32 commander ready\r\n");

    for (;;)
    {
        app_debug_process_rx();
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}

static void app_debug_process_rx(void)
{
    uint8_t byte;

    if (s_rx_overflow != 0U)
    {
        s_rx_overflow = 0U;
        app_debug_print("err: debug rx overflow\r\n");
    }

    while (xStreamBufferReceive(s_rx_stream, &byte, sizeof(byte), 0U) == sizeof(byte))
    {
        if (byte == ';')
        {
            if (s_command_len > 0U)
            {
                s_command[s_command_len] = '\0';
                app_debug_execute_command(s_command);
                s_command_len = 0U;
            }
        }
        else if ((byte == '\r') || (byte == '\n'))
        {
            /* Command terminator is ';'. CR/LF are just formatting noise. */
        }
        else if ((byte == 0x08U) || (byte == 0x7FU))
        {
            if (s_command_len > 0U)
            {
                s_command_len--;
            }
        }
        else if (s_command_len < (APP_DEBUG_COMMAND_MAX_LEN - 1U))
        {
            s_command[s_command_len++] = (char)byte;
        }
        else
        {
            s_command_len = 0U;
            app_debug_print("err: command too long\r\n");
        }
    }
}

static void app_debug_execute_command(char *command)
{
    char *argv[APP_DEBUG_ARG_MAX_COUNT];
    int argc = app_debug_split_args(command, argv, APP_DEBUG_ARG_MAX_COUNT);

    if (argc == 0)
    {
        return;
    }

    if (app_debug_streq(argv[0], "led") != 0U)
    {
        app_debug_handle_led_command(argc, argv);
    }
    else if ((argc == 1) && (app_debug_streq(argv[0], "show") != 0U))
    {
        app_debug_print_command_list();
    }
    else if (app_debug_streq(argv[0], "param") != 0U)
    {
        app_debug_handle_param_command(argc, argv);
    }
    else if (app_debug_streq(argv[0], "BN-220") != 0U)
    {
        app_debug_handle_gps_command(argc, argv);
    }
    else if (app_debug_streq(argv[0], "can") != 0U)
    {
        app_debug_handle_can_command(argc, argv);
    }
    else
    {
        app_debug_print("err: unknown command\r\n");
    }
}

static int app_debug_split_args(char *command, char **argv, int argv_max)
{
    int argc = 0;
    uint8_t in_token = 0U;

    if ((command == NULL) || (argv == NULL) || (argv_max <= 0))
    {
        return 0;
    }

    while (*command != '\0')
    {
        if ((*command == ' ') || (*command == '\t'))
        {
            *command = '\0';
            in_token = 0U;
        }
        else if (in_token == 0U)
        {
            if (argc >= argv_max)
            {
                return argc;
            }

            argv[argc++] = command;
            in_token = 1U;
        }

        command++;
    }

    return argc;
}

static void app_debug_handle_led_command(int argc, char **argv)
{
    app_led_control_msg_t control = {0};

    control.led_id = 0U;

    if (argc < 2)
    {
        app_debug_print_led_usage();
        return;
    }

    if (app_debug_streq(argv[1], "board") != 0U)
    {
        if ((argc == 4) &&
            (app_debug_streq(argv[2], "slave") != 0U) &&
            (app_debug_streq(argv[3], "request") != 0U))
        {
            app_debug_post_board_slave_led_request();
            return;
        }

        if (app_debug_parse_board_slave_led_command(argc, argv, &control) == 0U)
        {
            return;
        }

        app_debug_post_led_control(APP_MSG_BOARD_SLAVE_LED_CONTROL, &control);
        return;
    }

    if (app_debug_parse_local_led_command(argc, argv, &control) == 0U)
    {
        return;
    }

    app_debug_post_led_control(APP_MSG_LED_CONTROL, &control);
    app_debug_print("ok\r\n");
}

static void app_debug_handle_param_command(int argc, char **argv)
{
    if ((argc == 2) && (app_debug_streq(argv[1], "show") != 0U))
    {
        app_debug_print_param_table();
        return;
    }

    app_debug_print("usage: param show;\r\n");
}

static void app_debug_handle_gps_command(int argc, char **argv)
{
    if ((argc == 2) && (app_debug_streq(argv[1], "GPS") != 0U))
    {
        app_debug_print_gps_snapshot();
        return;
    }

    app_debug_print("usage: BN-220 GPS;\r\n");
}

static void app_debug_handle_can_command(int argc, char **argv)
{
    app_can_frame_msg_t frame = {0};
    uint32_t parsed;
    int index;

    if ((argc < 4) || (app_debug_streq(argv[1], "test") == 0U))
    {
        app_debug_print("usage: can test <id> <len> [data...];\r\n");
        return;
    }

    if ((app_debug_parse_u32_limit(argv[2], 0x7FFU, &parsed) == 0U))
    {
        app_debug_print("err: can id must be 0..0x7FF\r\n");
        return;
    }

    frame.std_id = (uint16_t)parsed;

    if ((app_debug_parse_u32_limit(argv[3], 8U, &parsed) == 0U) ||
        (argc != (4 + (int)parsed)))
    {
        app_debug_print("usage: can test <id> <len> [data...];\r\n");
        return;
    }

    frame.len = (uint8_t)parsed;

    for (index = 0; index < frame.len; index++)
    {
        if (app_debug_parse_u32_limit(argv[4 + index], 255U, &parsed) == 0U)
        {
            app_debug_print("err: can data byte must be 0..0xFF\r\n");
            return;
        }

        frame.data[index] = (uint8_t)parsed;
    }

    app_debug_post_can_debug_tx(&frame);
}

static void app_debug_print_command_list(void)
{
    app_debug_print(
        "commands:\r\n"
        "  show;\r\n"
        "  param show;\r\n"
        "  BN-220 GPS;\r\n"
        "  can test <id> <len> [data...];\r\n"
        "  led on|off|toggle;\r\n"
        "  led blink <hz> <count>;\r\n"
        "  led board slave on|off|toggle;\r\n"
        "  led board slave blink <hz> [count];\r\n"
        "  led board slave request;\r\n");
}

static void app_debug_print_led_usage(void)
{
    app_debug_print(
        "usage: led on|off|toggle;\r\n"
        "       led blink <hz> <count>;\r\n"
        "       led board slave on|off|toggle;\r\n"
        "       led board slave blink <hz> [count];\r\n"
        "       led board slave request;\r\n");
}

static void app_debug_print_board_slave_led_usage(void)
{
    app_debug_print(
        "usage: led board slave on|off|toggle;\r\n"
        "       led board slave blink <hz> [count];\r\n"
        "       led board slave request;\r\n");
}

static uint8_t app_debug_parse_local_led_command(int argc,
                                                 char **argv,
                                                 app_led_control_msg_t *control)
{
    if (app_debug_streq(argv[1], "blink") != 0U)
    {
        if (app_debug_parse_led_blink(argc, argv, 1, control) == 0U)
        {
            app_debug_print("usage: led blink <hz> <count>;\r\n");
            return 0U;
        }
    }
    else if (app_debug_parse_led_action(argv[1], &control->command) == 0U)
    {
        app_debug_print("err: expected on|off|toggle|blink\r\n");
        return 0U;
    }

    return 1U;
}

static uint8_t app_debug_parse_board_slave_led_command(
    int argc,
    char **argv,
    app_led_control_msg_t *control)
{
    if ((argc < 4) || (app_debug_streq(argv[2], "slave") == 0U))
    {
        app_debug_print_board_slave_led_usage();
        return 0U;
    }

    if (app_debug_streq(argv[3], "blink") != 0U)
    {
        if (app_debug_parse_led_blink_hz(argc, argv, 3, control) == 0U)
        {
            app_debug_print("usage: led board slave blink <hz> [count];\r\n");
            return 0U;
        }
    }
    else if ((argc != 4) ||
             (app_debug_parse_led_action(argv[3], &control->command) == 0U))
    {
        app_debug_print("err: expected on|off|toggle|blink\r\n");
        return 0U;
    }

    return 1U;
}

static uint8_t app_debug_parse_led_action(const char *token, uint8_t *command)
{
    if ((token == NULL) || (command == NULL))
    {
        return 0U;
    }

    if (app_debug_streq(token, "on") != 0U)
    {
        *command = LED_CMD_ON;
        return 1U;
    }

    if (app_debug_streq(token, "off") != 0U)
    {
        *command = LED_CMD_OFF;
        return 1U;
    }

    if (app_debug_streq(token, "toggle") != 0U)
    {
        *command = LED_CMD_TOGGLE;
        return 1U;
    }

    return 0U;
}

static uint8_t app_debug_parse_led_blink(int argc,
                                         char **argv,
                                         int start_index,
                                         app_led_control_msg_t *control)
{
    if ((control == NULL) || (argc != (start_index + 3)))
    {
        return 0U;
    }

    control->command = LED_CMD_BLINK_HZ_COUNT;

    if ((app_debug_parse_u16(argv[start_index + 1], &control->hz) == 0U) ||
        (app_debug_parse_u16(argv[start_index + 2], &control->count) == 0U))
    {
        return 0U;
    }

    return 1U;
}

static uint8_t app_debug_parse_led_blink_hz(int argc,
                                            char **argv,
                                            int start_index,
                                            app_led_control_msg_t *control)
{
    if ((control == NULL) ||
        ((argc != (start_index + 2)) && (argc != (start_index + 3))))
    {
        return 0U;
    }

    control->command = LED_CMD_BLINK_HZ_COUNT;
    control->count = 0U;

    if (app_debug_parse_u16(argv[start_index + 1], &control->hz) == 0U)
    {
        return 0U;
    }

    if (argc == (start_index + 3))
    {
        return app_debug_parse_u16(argv[start_index + 2], &control->count);
    }

    return 1U;
}

static uint8_t app_debug_parse_u16(const char *token, uint16_t *value)
{
    uint32_t parsed;

    if ((value == NULL) ||
        (app_debug_parse_u32_limit(token, 65535U, &parsed) == 0U))
    {
        return 0U;
    }

    *value = (uint16_t)parsed;
    return 1U;
}

static uint8_t app_debug_parse_u32_limit(const char *token,
                                         uint32_t max_value,
                                         uint32_t *value)
{
    uint32_t parsed = 0U;
    uint8_t base = 10U;
    uint8_t digit;

    if ((token == NULL) || (value == NULL))
    {
        return 0U;
    }

    if ((token[0] == '0') && ((token[1] == 'x') || (token[1] == 'X')))
    {
        base = 16U;
        token += 2;
    }

    if (*token == '\0')
    {
        return 0U;
    }

    while (*token != '\0')
    {
        if (base == 16U)
        {
            if (app_debug_ascii_to_nibble(*token, &digit) == 0U)
            {
                return 0U;
            }
        }
        else if ((*token >= '0') && (*token <= '9'))
        {
            digit = (uint8_t)(*token - '0');
        }
        else
        {
            return 0U;
        }

        if ((digit >= base) || (parsed > ((max_value - digit) / base)))
        {
            return 0U;
        }

        parsed = (parsed * base) + digit;
        token++;
    }

    *value = parsed;
    return 1U;
}

static uint8_t app_debug_ascii_to_nibble(char c, uint8_t *value)
{
    if ((c >= '0') && (c <= '9'))
    {
        *value = (uint8_t)(c - '0');
        return 1U;
    }

    if ((c >= 'a') && (c <= 'f'))
    {
        *value = (uint8_t)(c - 'a' + 10);
        return 1U;
    }

    if ((c >= 'A') && (c <= 'F'))
    {
        *value = (uint8_t)(c - 'A' + 10);
        return 1U;
    }

    return 0U;
}

static uint8_t app_debug_streq(const char *a, const char *b)
{
    if ((a == NULL) || (b == NULL))
    {
        return 0U;
    }

    while ((*a != '\0') && (*a == *b))
    {
        a++;
        b++;
    }

    return (uint8_t)(*a == *b);
}

static uint16_t app_debug_text_len(const char *text)
{
    uint16_t len = 0U;

    while ((len < UINT16_MAX) && (text[len] != '\0'))
    {
        len++;
    }

    return len;
}

static void app_debug_post_led_control(app_msg_id_t id,
                                       const app_led_control_msg_t *control)
{
    app_msg_t msg = {0};

    msg.id = id;
    msg.len = sizeof(msg.payload.led_control);
    msg.payload.led_control = *control;
    (void)app_task_post(&msg, 0U);
}

static void app_debug_post_board_slave_led_request(void)
{
    app_msg_t msg = {0};

    msg.id = APP_MSG_BOARD_SLAVE_LED_REQUEST;
    (void)app_task_post(&msg, 0U);
}

static void app_debug_post_can_debug_tx(const app_can_frame_msg_t *frame)
{
    app_msg_t msg = {0};

    msg.id = APP_MSG_CAN_DEBUG_TX;
    msg.len = sizeof(msg.payload.can_frame);
    msg.payload.can_frame = *frame;
    (void)app_task_post(&msg, 0U);
}

static void app_debug_print_param_table(void)
{
    param_id_t id;

    for (id = 0; id < PARAM_COUNT; id++)
    {
        app_debug_print(param_name(id));
        app_debug_print(" ");
        app_debug_write_i32(param_get(id));
        app_debug_print("\r\n");
    }
}

static void app_debug_print_gps_snapshot(void)
{
    app_gps_snapshot_t snapshot;

    app_gps_get_snapshot(&snapshot);

    app_debug_print("BN-220 GPS\r\n");
    app_debug_print("  data=");
    app_debug_print((snapshot.has_data != 0U) ? "yes" : "no");
    app_debug_print("\r\n");

    app_debug_print("  valid=");
    app_debug_print((snapshot.navigation_valid != 0U) ? "yes" : "no");
    app_debug_print("\r\n");

    app_debug_print("  utc_time=");
    app_debug_print_text_or_dash(snapshot.utc_time);
    app_debug_print("\r\n");

    app_debug_print("  utc_date=");
    app_debug_print_text_or_dash(snapshot.utc_date);
    app_debug_print("\r\n");

    app_debug_print("  latitude=");
    app_debug_print_text_or_dash(snapshot.latitude);
    app_debug_print("\r\n");

    app_debug_print("  latitude_hemi=");
    app_debug_print_text_or_dash(snapshot.latitude_hemi);
    app_debug_print("\r\n");

    app_debug_print("  longitude=");
    app_debug_print_text_or_dash(snapshot.longitude);
    app_debug_print("\r\n");

    app_debug_print("  longitude_hemi=");
    app_debug_print_text_or_dash(snapshot.longitude_hemi);
    app_debug_print("\r\n");

    app_debug_print("  speed_knots=");
    app_debug_print_text_or_dash(snapshot.speed_knots);
    app_debug_print("\r\n");

    app_debug_print("  course_deg=");
    app_debug_print_text_or_dash(snapshot.course_deg);
    app_debug_print("\r\n");

    app_debug_print("  altitude_m=");
    app_debug_print_text_or_dash(snapshot.altitude_m);
    app_debug_print("\r\n");

    app_debug_print("  fix_quality=");
    app_debug_print_text_or_dash(snapshot.fix_quality);
    app_debug_print("\r\n");

    app_debug_print("  fix_type=");
    app_debug_print_text_or_dash(snapshot.fix_type);
    app_debug_print("\r\n");

    app_debug_print("  sats_used=");
    app_debug_print_text_or_dash(snapshot.satellites_used);
    app_debug_print("\r\n");

    app_debug_print("  sats_view=");
    app_debug_print_text_or_dash(snapshot.satellites_in_view);
    app_debug_print("\r\n");

    app_debug_print("  pdop=");
    app_debug_print_text_or_dash(snapshot.pdop);
    app_debug_print("\r\n");

    app_debug_print("  hdop=");
    app_debug_print_text_or_dash(snapshot.hdop);
    app_debug_print("\r\n");

    app_debug_print("  vdop=");
    app_debug_print_text_or_dash(snapshot.vdop);
    app_debug_print("\r\n");

    app_debug_print("  sentences_ok=");
    app_debug_write_u32(snapshot.valid_sentence_count);
    app_debug_print("\r\n");

    app_debug_print("  checksum_err=");
    app_debug_write_u32(snapshot.checksum_error_count);
    app_debug_print("\r\n");
}

static void app_debug_print_text_or_dash(const char *text)
{
    if ((text == NULL) || (text[0] == '\0'))
    {
        app_debug_print("--");
        return;
    }

    app_debug_print(text);
}

static void app_debug_print_can_error_detail(uint32_t error_code)
{
    app_debug_print_can_error_group(
        error_code,
        BOARD_SERVICE_CAN_ERROR_WARNING |
        BOARD_SERVICE_CAN_ERROR_PASSIVE |
        BOARD_SERVICE_CAN_ERROR_BUS_OFF,
        " state=");

    app_debug_print_can_error_group(
        error_code,
        BOARD_SERVICE_CAN_ERROR_STUFF |
        BOARD_SERVICE_CAN_ERROR_FORM |
        BOARD_SERVICE_CAN_ERROR_ACK |
        BOARD_SERVICE_CAN_ERROR_BIT_RECESSIVE |
        BOARD_SERVICE_CAN_ERROR_BIT_DOMINANT |
        BOARD_SERVICE_CAN_ERROR_CRC,
        " cause=");

    app_debug_print_can_error_group(
        error_code,
        BOARD_SERVICE_CAN_ERROR_RX_OVERRUN0 |
        BOARD_SERVICE_CAN_ERROR_RX_OVERRUN1 |
        BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST0 |
        BOARD_SERVICE_CAN_ERROR_TX0 |
        BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST1 |
        BOARD_SERVICE_CAN_ERROR_TX1 |
        BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST2 |
        BOARD_SERVICE_CAN_ERROR_TX2,
        " detail=");

    app_debug_print_can_error_group(
        error_code,
        BOARD_SERVICE_CAN_ERROR_TIMEOUT |
        BOARD_SERVICE_CAN_ERROR_NOT_INITIALIZED |
        BOARD_SERVICE_CAN_ERROR_NOT_READY |
        BOARD_SERVICE_CAN_ERROR_NOT_STARTED |
        BOARD_SERVICE_CAN_ERROR_PARAM |
        BOARD_SERVICE_CAN_ERROR_INTERNAL,
        " driver=");
}

static void app_debug_print_can_error_group(uint32_t error_code,
                                            uint32_t group_mask,
                                            const char *label)
{
    uint8_t printed = 0U;

    if ((error_code & group_mask) == 0U)
    {
        return;
    }

    app_debug_print(label);

#define APP_DEBUG_CAN_PRINT_ERROR(flag, text)                \
    do                                                       \
    {                                                        \
        if (((group_mask & (flag)) != 0U) &&                 \
            ((error_code & (flag)) != 0U))                   \
        {                                                    \
            if (printed != 0U)                               \
            {                                                \
                app_debug_print("|");                        \
            }                                                \
            app_debug_print((text));                         \
            printed = 1U;                                    \
        }                                                    \
    } while (0)

    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_WARNING, "warning");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_PASSIVE, "passive");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_BUS_OFF, "bus_off");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_STUFF, "stuff");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_FORM, "form");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_ACK, "ack");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_BIT_RECESSIVE, "bit_recessive");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_BIT_DOMINANT, "bit_dominant");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_CRC, "crc");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_RX_OVERRUN0, "rx_overrun0");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_RX_OVERRUN1, "rx_overrun1");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST0,
                              "tx_arbitration_lost0");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_TX0, "tx_error0");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST1,
                              "tx_arbitration_lost1");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_TX1, "tx_error1");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_TX_ARBITRATION_LOST2,
                              "tx_arbitration_lost2");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_TX2, "tx_error2");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_TIMEOUT, "timeout");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_NOT_INITIALIZED, "not_initialized");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_NOT_READY, "not_ready");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_NOT_STARTED, "not_started");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_PARAM, "param");
    APP_DEBUG_CAN_PRINT_ERROR(BOARD_SERVICE_CAN_ERROR_INTERNAL, "internal");

#undef APP_DEBUG_CAN_PRINT_ERROR
}

static void app_debug_write_i32(int32_t value)
{
    uint32_t magnitude;

    if (value < 0)
    {
        app_debug_print("-");
        magnitude = (uint32_t)(-(value + 1)) + 1U;
    }
    else
    {
        magnitude = (uint32_t)value;
    }

    app_debug_write_u32(magnitude);
}

static void app_debug_write_u32(uint32_t value)
{
    char text[10];
    uint8_t pos = 0U;

    do
    {
        text[pos++] = (char)('0' + (value % 10U));
        value /= 10U;
    } while ((value != 0U) && (pos < sizeof(text)));

    while (pos > 0U)
    {
        pos--;
        board_service_console_write((const uint8_t *)&text[pos], 1U);
    }
}
