#include "app_debug.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

#include "app_task.h"
#include "debug_port.h"
#include "param.h"

#define APP_DEBUG_RX_BUFFER_SIZE       128U
#define APP_DEBUG_COMMAND_MAX_LEN      80U
#define APP_DEBUG_ARG_MAX_COUNT        8U
#define APP_DEBUG_WRITE_TIMEOUT_MS     50U

static StreamBufferHandle_t s_rx_stream;
static char s_command[APP_DEBUG_COMMAND_MAX_LEN];
static uint8_t s_command_len;
static uint8_t s_rx_overflow;

static void app_debug_process_rx(void);
static void app_debug_execute_command(char *command);
static void app_debug_handle_led_command(int argc, char **argv);
static void app_debug_handle_param_command(int argc, char **argv);
static uint8_t app_debug_parse_led_action(const char *token, uint8_t *command);
static uint8_t app_debug_parse_led_blink(int argc,
                                         char **argv,
                                         int start_index,
                                         app_led_control_msg_t *control);
static uint8_t app_debug_parse_led_blink_hz(int argc,
                                            char **argv,
                                            int start_index,
                                            app_led_control_msg_t *control);
static uint8_t app_debug_parse_i32(const char *token, int32_t *value);
static uint8_t app_debug_parse_u16(const char *token, uint16_t *value);
static uint8_t app_debug_parse_u32_limit(const char *token,
                                         uint32_t max_value,
                                         uint32_t *value);
static uint8_t app_debug_ascii_to_nibble(char c, uint8_t *value);
static uint8_t app_debug_streq(const char *a, const char *b);
static void app_debug_post_local_led_control(const app_led_control_msg_t *control);
static void app_debug_post_board_slave_led_control(
    const app_led_control_msg_t *control);
static void app_debug_print_param_table(void);
static void app_debug_write_i32(int32_t value);
static void app_debug_write_u32(uint32_t value);

void app_debug_init(void)
{
    bsp_debug_init();
    s_command_len = 0U;
    s_rx_overflow = 0U;
    s_rx_stream = xStreamBufferCreate(APP_DEBUG_RX_BUFFER_SIZE, 1U);
    configASSERT(s_rx_stream != NULL);
}

void app_debug_print(const char *text)
{
    uint16_t len = 0U;

    if (text == NULL)
    {
        return;
    }

    while ((text[len] != '\0') && (len < UINT16_MAX))
    {
        len++;
    }

    if (len > 0U)
    {
        bsp_debug_write((const uint8_t *)text, len);
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
    uint16_t write_len;

    configASSERT(msg != NULL);

    switch (msg->id)
    {
        case APP_MSG_DEBUG_PRINT:
        case APP_MSG_DEBUG_UART_RX_FRAME:
            write_len = msg->len;
            if (write_len > APP_DEBUG_DATA_MAX_LEN)
            {
                write_len = APP_DEBUG_DATA_MAX_LEN;
            }

            if (write_len > 0U)
            {
                bsp_debug_write(msg->payload.debug.data, write_len);
            }
            break;

        case APP_MSG_DEBUG_ERROR:
            app_debug_print("debug error\r\n");
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
    int argc = 0;
    char *token;

    token = strtok(command, " \t");

    while ((token != NULL) && (argc < APP_DEBUG_ARG_MAX_COUNT))
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t");
    }

    if (argc == 0)
    {
        return;
    }

    if (app_debug_streq(argv[0], "led") != 0U)
    {
        app_debug_handle_led_command(argc, argv);
    }
    else if (app_debug_streq(argv[0], "param") != 0U)
    {
        app_debug_handle_param_command(argc, argv);
    }
    else
    {
        app_debug_print("err: unknown command\r\n");
    }
}

static void app_debug_handle_led_command(int argc, char **argv)
{
    app_led_control_msg_t control = {0};

    control.led_id = 0U;

    if (argc < 2)
    {
        app_debug_print("usage: led on|off|toggle;\r\n");
        app_debug_print("       led blink <hz> <count>;\r\n");
        app_debug_print("       led board slave on|off|toggle;\r\n");
        app_debug_print("       led board slave blink <hz>;\r\n");
        return;
    }

    if (app_debug_streq(argv[1], "board") != 0U)
    {
        if ((argc < 4) || (app_debug_streq(argv[2], "slave") == 0U))
        {
            app_debug_print("usage: led board slave on|off|toggle;\r\n");
            app_debug_print("       led board slave blink <hz>;\r\n");
            return;
        }

        if (app_debug_streq(argv[3], "blink") != 0U)
        {
            if (app_debug_parse_led_blink_hz(argc, argv, 3, &control) == 0U)
            {
                app_debug_print("usage: led board slave blink <hz>;\r\n");
                return;
            }
        }
        else if ((argc != 4) ||
                 (app_debug_parse_led_action(argv[3], &control.command) == 0U))
        {
            app_debug_print("err: expected on|off|toggle|blink\r\n");
            return;
        }

        app_debug_post_board_slave_led_control(&control);
        return;
    }

    if (app_debug_streq(argv[1], "blink") != 0U)
    {
        if (app_debug_parse_led_blink(argc, argv, 1, &control) == 0U)
        {
            app_debug_print("usage: led blink <hz> <count>;\r\n");
            return;
        }
    }
    else if (app_debug_parse_led_action(argv[1], &control.command) == 0U)
    {
        app_debug_print("err: expected on|off|toggle|blink\r\n");
        return;
    }

    app_debug_post_local_led_control(&control);
    app_debug_print("ok\r\n");
}

static void app_debug_handle_param_command(int argc, char **argv)
{
    app_msg_t msg = {0};
    int32_t value;
    param_id_t id;

    if ((argc == 2) && (app_debug_streq(argv[1], "show") != 0U))
    {
        app_debug_print_param_table();
        return;
    }

    if ((argc != 4) || (app_debug_streq(argv[1], "set") == 0U))
    {
        app_debug_print("usage: param set <name> <value>;\r\n");
        app_debug_print("       param show;\r\n");
        return;
    }

    if ((strlen(argv[2]) >= APP_PARAM_NAME_MAX_LEN) ||
        (param_find(argv[2], &id) == 0U) ||
        (app_debug_parse_i32(argv[3], &value) == 0U))
    {
        app_debug_print("err: bad param command\r\n");
        return;
    }

    msg.id = APP_MSG_PARAM_UPDATE;
    msg.len = sizeof(msg.payload.param_update);
    (void)strncpy(msg.payload.param_update.name,
                  argv[2],
                  sizeof(msg.payload.param_update.name) - 1U);
    msg.payload.param_update.value = value;
    (void)app_task_post(&msg, 0U);
    (void)id;
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
    if ((control == NULL) || (argc != (start_index + 2)))
    {
        return 0U;
    }

    control->command = LED_CMD_BLINK_HZ_COUNT;
    control->count = 0U;

    return app_debug_parse_u16(argv[start_index + 1], &control->hz);
}

static uint8_t app_debug_parse_i32(const char *token, int32_t *value)
{
    uint32_t parsed;
    uint8_t negative = 0U;
    uint32_t limit = INT32_MAX;

    if ((token == NULL) || (value == NULL))
    {
        return 0U;
    }

    if (*token == '-')
    {
        negative = 1U;
        token++;
        limit = 2147483648UL;
    }

    if (app_debug_parse_u32_limit(token, limit, &parsed) == 0U)
    {
        return 0U;
    }

    if (negative != 0U)
    {
        *value = (parsed == 2147483648UL) ? INT32_MIN : -(int32_t)parsed;
    }
    else
    {
        *value = (int32_t)parsed;
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
    return (uint8_t)(strcmp(a, b) == 0);
}

static void app_debug_post_local_led_control(const app_led_control_msg_t *control)
{
    app_msg_t msg = {0};

    msg.id = APP_MSG_LED_CONTROL;
    msg.len = sizeof(msg.payload.led_control);
    msg.payload.led_control = *control;
    (void)app_task_post(&msg, 0U);
}

static void app_debug_post_board_slave_led_control(
    const app_led_control_msg_t *control)
{
    app_msg_t msg = {0};

    msg.id = APP_MSG_BOARD_SLAVE_LED_CONTROL;
    msg.len = sizeof(msg.payload.led_control);
    msg.payload.led_control = *control;
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
        (void)console_port_write((const uint8_t *)&text[pos],
                                 1U,
                                 APP_DEBUG_WRITE_TIMEOUT_MS);
    }
}
