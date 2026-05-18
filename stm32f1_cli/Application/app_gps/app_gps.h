#ifndef APP_GPS_H
#define APP_GPS_H

#include <stdint.h>

#define APP_GPS_UTC_TIME_LEN          12U
#define APP_GPS_UTC_DATE_LEN           8U
#define APP_GPS_COORDINATE_LEN        16U
#define APP_GPS_SHORT_FIELD_LEN        8U
#define APP_GPS_VALUE_FIELD_LEN       12U

typedef struct
{
    uint8_t has_data;
    uint8_t navigation_valid;
    uint32_t valid_sentence_count;
    uint32_t checksum_error_count;

    char utc_time[APP_GPS_UTC_TIME_LEN];
    char utc_date[APP_GPS_UTC_DATE_LEN];
    char latitude[APP_GPS_COORDINATE_LEN];
    char latitude_hemi[APP_GPS_SHORT_FIELD_LEN];
    char longitude[APP_GPS_COORDINATE_LEN];
    char longitude_hemi[APP_GPS_SHORT_FIELD_LEN];
    char speed_knots[APP_GPS_VALUE_FIELD_LEN];
    char course_deg[APP_GPS_VALUE_FIELD_LEN];
    char fix_quality[APP_GPS_SHORT_FIELD_LEN];
    char fix_type[APP_GPS_SHORT_FIELD_LEN];
    char satellites_used[APP_GPS_SHORT_FIELD_LEN];
    char satellites_in_view[APP_GPS_SHORT_FIELD_LEN];
    char pdop[APP_GPS_VALUE_FIELD_LEN];
    char hdop[APP_GPS_VALUE_FIELD_LEN];
    char vdop[APP_GPS_VALUE_FIELD_LEN];
    char altitude_m[APP_GPS_VALUE_FIELD_LEN];
} app_gps_snapshot_t;

void app_gps_init(void);
void app_gps_rx_byte_from_isr(uint8_t byte);
void app_gps_get_snapshot(app_gps_snapshot_t *snapshot);
void app_gps_task_entry(void *argument);

#endif /* APP_GPS_H */
