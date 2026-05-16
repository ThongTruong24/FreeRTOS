#ifndef APP_MESSAGE_H
#define APP_MESSAGE_H

#include <stdint.h>

/*
 * The old master used topic_id_t as a second routing concept beside messages.
 * The new architecture uses app_msg_id_t directly for both in-process routing
 * and protocol frames that cross a device link.
 *
 * Wire-visible IDs intentionally keep the old numeric values that are still in
 * use so existing slaves can be migrated without changing the bus contract.
 */
typedef enum
{
    APP_MSG_LED_CONTROL = 0,
    APP_MSG_PARAM_UPDATE = 3,
    APP_MSG_BOARD_CONFIG = 4,

    /* Internal-only application events. */
    APP_MSG_SYSTEM_START = 64,
    APP_MSG_SYSTEM_ERROR,
    APP_MSG_LED_TICK,
    APP_MSG_BOARD_SLAVE_LED_CONTROL,
    APP_MSG_DEBUG_PRINT,
    APP_MSG_DEBUG_UART_RX_FRAME,
    APP_MSG_DEBUG_ERROR,

    APP_MSG_NONE = 0xFE,
    APP_MSG_MAX = 0xFF
} app_msg_id_t;

#define APP_DEBUG_DATA_MAX_LEN          64U
#define APP_PROTOCOL_PAYLOAD_MAX_LEN    64U
#define APP_PARAM_NAME_MAX_LEN           16U

typedef enum
{
    BOARD_BUS_I2C = 0,
    BOARD_BUS_SPI,
    BOARD_BUS_UART
} board_bus_t;

typedef enum
{
    BOARD_CONFIG_DISABLE = 0,
    BOARD_CONFIG_ENABLE
} board_config_action_t;

typedef enum
{
    LED_CMD_OFF = 0,
    LED_CMD_ON,
    LED_CMD_TOGGLE,
    LED_CMD_BLINK,
    LED_CMD_BLINK_HZ_COUNT
} led_cmd_t;

typedef enum
{
    LED_STATE_OFF = 0,
    LED_STATE_ON = 1
} led_state_t;

typedef struct
{
    uint8_t led_id;
    uint8_t command;
    uint16_t period_ms;
    uint16_t hz;
    uint16_t count;
} app_led_control_msg_t;

#define APP_LED_CONTROL_MSG_SIZE 8U

typedef struct
{
    board_bus_t bus;
    uint8_t instance;
    board_config_action_t action;
} app_board_config_msg_t;

typedef struct
{
    char name[APP_PARAM_NAME_MAX_LEN];
    int32_t value;
} app_param_update_msg_t;

typedef struct
{
    app_msg_id_t id;
    uint16_t len;
    uint32_t timestamp_ms;

    union
    {
        app_led_control_msg_t led_control;
        app_board_config_msg_t board_config;
        app_param_update_msg_t param_update;

        struct
        {
            uint8_t data[APP_DEBUG_DATA_MAX_LEN];
        } debug;

        struct
        {
            uint32_t code;
        } error;
    } payload;
} app_msg_t;

typedef char app_led_control_msg_size_check[
    (sizeof(app_led_control_msg_t) == APP_LED_CONTROL_MSG_SIZE) ? 1 : -1];

#endif /* APP_MESSAGE_H */
