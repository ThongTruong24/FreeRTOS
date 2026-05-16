#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#include "message.h"

#define PROTOCOL_START_BYTE      0xFAU
#define PROTOCOL_FRAME_MAX_LEN   (3U + APP_PROTOCOL_PAYLOAD_MAX_LEN)

typedef struct
{
    app_msg_id_t message_id;
    uint8_t len;
    uint8_t payload[APP_PROTOCOL_PAYLOAD_MAX_LEN];
} protocol_frame_t;

typedef enum
{
    PROTOCOL_PARSE_WAIT_START = 0,
    PROTOCOL_PARSE_MESSAGE_ID,
    PROTOCOL_PARSE_LEN,
    PROTOCOL_PARSE_PAYLOAD
} protocol_parse_state_t;

typedef struct
{
    protocol_parse_state_t state;
    protocol_frame_t frame;
    uint8_t index;
} protocol_parser_t;

void protocol_parser_init(protocol_parser_t *parser);
uint8_t protocol_parse_byte(protocol_parser_t *parser, uint8_t byte, protocol_frame_t *out);
uint8_t protocol_encode_frame(app_msg_id_t message_id,
                              const uint8_t *payload,
                              uint8_t len,
                              uint8_t *out,
                              uint8_t out_max);

#endif /* PROTOCOL_H */
