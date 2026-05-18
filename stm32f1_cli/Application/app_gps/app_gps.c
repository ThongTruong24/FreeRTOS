#include "app_gps.h"

#include <string.h>

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "task.h"

#define APP_GPS_RX_BUFFER_SIZE          256U
#define APP_GPS_SENTENCE_MAX_LEN         96U
#define APP_GPS_FIELD_MAX_COUNT          20U

static StreamBufferHandle_t s_rx_stream;
static uint8_t s_rx_overflow;
static char s_sentence[APP_GPS_SENTENCE_MAX_LEN];
static uint8_t s_sentence_len;
static app_gps_snapshot_t s_snapshot;

static void app_gps_process_rx(void);
static void app_gps_finish_sentence(void);
static uint8_t app_gps_checksum_is_valid(const char *sentence);
static uint8_t app_gps_hex_to_nibble(char c, uint8_t *value);
static uint8_t app_gps_split_fields(char *sentence, char **fields, uint8_t fields_max);
static void app_gps_parse_sentence(char *sentence);
static void app_gps_parse_rmc(char **fields, uint8_t count);
static void app_gps_parse_vtg(char **fields, uint8_t count);
static void app_gps_parse_gga(char **fields, uint8_t count);
static void app_gps_parse_gsa(char **fields, uint8_t count);
static void app_gps_parse_gsv(char **fields, uint8_t count);
static void app_gps_parse_gll(char **fields, uint8_t count);
static uint8_t app_gps_sentence_is(const char *field, const char *suffix);
static void app_gps_copy_text(char *dst, uint8_t dst_len, const char *src);
static void app_gps_copy_if_present(char *dst, uint8_t dst_len, const char *src);
static uint8_t app_gps_text_is_empty(const char *text);

void app_gps_init(void)
{
    memset(&s_snapshot, 0, sizeof(s_snapshot));
    s_sentence_len = 0U;
    s_rx_overflow = 0U;
    s_rx_stream = xStreamBufferCreate(APP_GPS_RX_BUFFER_SIZE, 1U);
    configASSERT(s_rx_stream != NULL);
}

void app_gps_rx_byte_from_isr(uint8_t byte)
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

void app_gps_get_snapshot(app_gps_snapshot_t *snapshot)
{
    if (snapshot == NULL)
    {
        return;
    }

    taskENTER_CRITICAL();
    *snapshot = s_snapshot;
    taskEXIT_CRITICAL();
}

void app_gps_task_entry(void *argument)
{
    (void)argument;

    for (;;)
    {
        app_gps_process_rx();
        vTaskDelay(pdMS_TO_TICKS(5U));
    }
}

static void app_gps_process_rx(void)
{
    uint8_t byte;

    if (s_rx_overflow != 0U)
    {
        s_rx_overflow = 0U;
        s_sentence_len = 0U;
    }

    while (xStreamBufferReceive(s_rx_stream, &byte, sizeof(byte), 0U) == sizeof(byte))
    {
        if (byte == '$')
        {
            s_sentence_len = 0U;
            s_sentence[s_sentence_len++] = (char)byte;
        }
        else if ((byte == '\r') || (byte == '\n'))
        {
            if (s_sentence_len > 0U)
            {
                app_gps_finish_sentence();
            }
        }
        else if ((s_sentence_len > 0U) &&
                 (s_sentence_len < (APP_GPS_SENTENCE_MAX_LEN - 1U)))
        {
            s_sentence[s_sentence_len++] = (char)byte;
        }
        else if (s_sentence_len >= (APP_GPS_SENTENCE_MAX_LEN - 1U))
        {
            s_sentence_len = 0U;
        }
    }
}

static void app_gps_finish_sentence(void)
{
    s_sentence[s_sentence_len] = '\0';
    s_sentence_len = 0U;

    if (app_gps_checksum_is_valid(s_sentence) == 0U)
    {
        taskENTER_CRITICAL();
        s_snapshot.checksum_error_count++;
        taskEXIT_CRITICAL();
        return;
    }

    app_gps_parse_sentence(s_sentence);
}

static uint8_t app_gps_checksum_is_valid(const char *sentence)
{
    const char *cursor;
    uint8_t checksum = 0U;
    uint8_t expected_high;
    uint8_t expected_low;

    if ((sentence == NULL) || (sentence[0] != '$'))
    {
        return 0U;
    }

    cursor = &sentence[1];

    while ((*cursor != '\0') && (*cursor != '*'))
    {
        checksum ^= (uint8_t)*cursor;
        cursor++;
    }

    if ((*cursor != '*') ||
        (app_gps_hex_to_nibble(cursor[1], &expected_high) == 0U) ||
        (app_gps_hex_to_nibble(cursor[2], &expected_low) == 0U) ||
        (cursor[3] != '\0'))
    {
        return 0U;
    }

    return (uint8_t)(checksum == (uint8_t)((expected_high << 4) | expected_low));
}

static uint8_t app_gps_hex_to_nibble(char c, uint8_t *value)
{
    if (value == NULL)
    {
        return 0U;
    }

    if ((c >= '0') && (c <= '9'))
    {
        *value = (uint8_t)(c - '0');
        return 1U;
    }

    if ((c >= 'A') && (c <= 'F'))
    {
        *value = (uint8_t)(c - 'A' + 10);
        return 1U;
    }

    if ((c >= 'a') && (c <= 'f'))
    {
        *value = (uint8_t)(c - 'a' + 10);
        return 1U;
    }

    return 0U;
}

static uint8_t app_gps_split_fields(char *sentence, char **fields, uint8_t fields_max)
{
    uint8_t count = 0U;
    char *cursor;

    if ((sentence == NULL) || (fields == NULL) || (fields_max == 0U))
    {
        return 0U;
    }

    cursor = sentence;

    if (*cursor == '$')
    {
        cursor++;
    }

    fields[count++] = cursor;

    while ((*cursor != '\0') && (count < fields_max))
    {
        if (*cursor == '*')
        {
            *cursor = '\0';
            break;
        }

        if (*cursor == ',')
        {
            *cursor = '\0';
            fields[count++] = cursor + 1;
        }

        cursor++;
    }

    return count;
}

static void app_gps_parse_sentence(char *sentence)
{
    char *fields[APP_GPS_FIELD_MAX_COUNT];
    uint8_t count;

    count = app_gps_split_fields(sentence, fields, APP_GPS_FIELD_MAX_COUNT);

    if (count == 0U)
    {
        return;
    }

    taskENTER_CRITICAL();
    s_snapshot.has_data = 1U;
    s_snapshot.valid_sentence_count++;
    taskEXIT_CRITICAL();

    if (app_gps_sentence_is(fields[0], "RMC") != 0U)
    {
        app_gps_parse_rmc(fields, count);
    }
    else if (app_gps_sentence_is(fields[0], "VTG") != 0U)
    {
        app_gps_parse_vtg(fields, count);
    }
    else if (app_gps_sentence_is(fields[0], "GGA") != 0U)
    {
        app_gps_parse_gga(fields, count);
    }
    else if (app_gps_sentence_is(fields[0], "GSA") != 0U)
    {
        app_gps_parse_gsa(fields, count);
    }
    else if (app_gps_sentence_is(fields[0], "GSV") != 0U)
    {
        app_gps_parse_gsv(fields, count);
    }
    else if (app_gps_sentence_is(fields[0], "GLL") != 0U)
    {
        app_gps_parse_gll(fields, count);
    }
}

static void app_gps_parse_rmc(char **fields, uint8_t count)
{
    if (count < 10U)
    {
        return;
    }

    taskENTER_CRITICAL();
    app_gps_copy_if_present(s_snapshot.utc_time, sizeof(s_snapshot.utc_time), fields[1]);
    s_snapshot.navigation_valid = (uint8_t)(fields[2][0] == 'A');
    app_gps_copy_if_present(s_snapshot.latitude, sizeof(s_snapshot.latitude), fields[3]);
    app_gps_copy_if_present(s_snapshot.latitude_hemi, sizeof(s_snapshot.latitude_hemi), fields[4]);
    app_gps_copy_if_present(s_snapshot.longitude, sizeof(s_snapshot.longitude), fields[5]);
    app_gps_copy_if_present(s_snapshot.longitude_hemi, sizeof(s_snapshot.longitude_hemi), fields[6]);
    app_gps_copy_if_present(s_snapshot.speed_knots, sizeof(s_snapshot.speed_knots), fields[7]);
    app_gps_copy_if_present(s_snapshot.course_deg, sizeof(s_snapshot.course_deg), fields[8]);
    app_gps_copy_if_present(s_snapshot.utc_date, sizeof(s_snapshot.utc_date), fields[9]);
    taskEXIT_CRITICAL();
}

static void app_gps_parse_vtg(char **fields, uint8_t count)
{
    if (count < 6U)
    {
        return;
    }

    taskENTER_CRITICAL();
    app_gps_copy_if_present(s_snapshot.course_deg, sizeof(s_snapshot.course_deg), fields[1]);
    app_gps_copy_if_present(s_snapshot.speed_knots, sizeof(s_snapshot.speed_knots), fields[5]);
    taskEXIT_CRITICAL();
}

static void app_gps_parse_gga(char **fields, uint8_t count)
{
    if (count < 10U)
    {
        return;
    }

    taskENTER_CRITICAL();
    app_gps_copy_if_present(s_snapshot.utc_time, sizeof(s_snapshot.utc_time), fields[1]);
    app_gps_copy_if_present(s_snapshot.latitude, sizeof(s_snapshot.latitude), fields[2]);
    app_gps_copy_if_present(s_snapshot.latitude_hemi, sizeof(s_snapshot.latitude_hemi), fields[3]);
    app_gps_copy_if_present(s_snapshot.longitude, sizeof(s_snapshot.longitude), fields[4]);
    app_gps_copy_if_present(s_snapshot.longitude_hemi, sizeof(s_snapshot.longitude_hemi), fields[5]);
    app_gps_copy_text(s_snapshot.fix_quality, sizeof(s_snapshot.fix_quality), fields[6]);
    app_gps_copy_text(s_snapshot.satellites_used, sizeof(s_snapshot.satellites_used), fields[7]);
    app_gps_copy_if_present(s_snapshot.hdop, sizeof(s_snapshot.hdop), fields[8]);
    app_gps_copy_if_present(s_snapshot.altitude_m, sizeof(s_snapshot.altitude_m), fields[9]);
    taskEXIT_CRITICAL();
}

static void app_gps_parse_gsa(char **fields, uint8_t count)
{
    if (count < 18U)
    {
        return;
    }

    taskENTER_CRITICAL();
    app_gps_copy_text(s_snapshot.fix_type, sizeof(s_snapshot.fix_type), fields[2]);
    app_gps_copy_if_present(s_snapshot.pdop, sizeof(s_snapshot.pdop), fields[15]);
    app_gps_copy_if_present(s_snapshot.hdop, sizeof(s_snapshot.hdop), fields[16]);
    app_gps_copy_if_present(s_snapshot.vdop, sizeof(s_snapshot.vdop), fields[17]);
    taskEXIT_CRITICAL();
}

static void app_gps_parse_gsv(char **fields, uint8_t count)
{
    if (count < 4U)
    {
        return;
    }

    taskENTER_CRITICAL();
    app_gps_copy_text(s_snapshot.satellites_in_view,
                      sizeof(s_snapshot.satellites_in_view),
                      fields[3]);
    taskEXIT_CRITICAL();
}

static void app_gps_parse_gll(char **fields, uint8_t count)
{
    if (count < 7U)
    {
        return;
    }

    taskENTER_CRITICAL();
    app_gps_copy_if_present(s_snapshot.latitude, sizeof(s_snapshot.latitude), fields[1]);
    app_gps_copy_if_present(s_snapshot.latitude_hemi, sizeof(s_snapshot.latitude_hemi), fields[2]);
    app_gps_copy_if_present(s_snapshot.longitude, sizeof(s_snapshot.longitude), fields[3]);
    app_gps_copy_if_present(s_snapshot.longitude_hemi, sizeof(s_snapshot.longitude_hemi), fields[4]);
    app_gps_copy_if_present(s_snapshot.utc_time, sizeof(s_snapshot.utc_time), fields[5]);

    if (app_gps_text_is_empty(fields[6]) == 0U)
    {
        s_snapshot.navigation_valid = (uint8_t)(fields[6][0] == 'A');
    }

    taskEXIT_CRITICAL();
}

static uint8_t app_gps_sentence_is(const char *field, const char *suffix)
{
    uint8_t field_len = 0U;
    uint8_t suffix_len = 0U;

    if ((field == NULL) || (suffix == NULL))
    {
        return 0U;
    }

    while (field[field_len] != '\0')
    {
        field_len++;
    }

    while (suffix[suffix_len] != '\0')
    {
        suffix_len++;
    }

    if ((field_len < suffix_len) || (suffix_len == 0U))
    {
        return 0U;
    }

    return (uint8_t)(strcmp(&field[field_len - suffix_len], suffix) == 0);
}

static void app_gps_copy_text(char *dst, uint8_t dst_len, const char *src)
{
    uint8_t index = 0U;

    if ((dst == NULL) || (dst_len == 0U))
    {
        return;
    }

    if (src == NULL)
    {
        dst[0] = '\0';
        return;
    }

    while ((index < (uint8_t)(dst_len - 1U)) && (src[index] != '\0'))
    {
        dst[index] = src[index];
        index++;
    }

    dst[index] = '\0';
}

static void app_gps_copy_if_present(char *dst, uint8_t dst_len, const char *src)
{
    if (app_gps_text_is_empty(src) == 0U)
    {
        app_gps_copy_text(dst, dst_len, src);
    }
}

static uint8_t app_gps_text_is_empty(const char *text)
{
    return (uint8_t)((text == NULL) || (text[0] == '\0'));
}
