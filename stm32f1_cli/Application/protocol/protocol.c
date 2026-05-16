#include "protocol.h"

#include <string.h>

void protocol_parser_init(protocol_parser_t *parser)
{
    if (parser == 0)
    {
        return;
    }

    memset(parser, 0, sizeof(*parser));
    parser->state = PROTOCOL_PARSE_WAIT_START;
}

uint8_t protocol_parse_byte(protocol_parser_t *parser, uint8_t byte, protocol_frame_t *out)
{
    if ((parser == 0) || (out == 0))
    {
        return 0U;
    }

    switch (parser->state)
    {
        case PROTOCOL_PARSE_WAIT_START:
            if (byte == PROTOCOL_START_BYTE)
            {
                parser->state = PROTOCOL_PARSE_MESSAGE_ID;
                parser->index = 0U;
            }
            break;

        case PROTOCOL_PARSE_MESSAGE_ID:
            parser->frame.message_id = (app_msg_id_t)byte;
            parser->state = PROTOCOL_PARSE_LEN;
            break;

        case PROTOCOL_PARSE_LEN:
            parser->frame.len = byte;
            parser->index = 0U;

            if (parser->frame.len > APP_PROTOCOL_PAYLOAD_MAX_LEN)
            {
                parser->state = PROTOCOL_PARSE_WAIT_START;
            }
            else if (parser->frame.len == 0U)
            {
                *out = parser->frame;
                parser->state = PROTOCOL_PARSE_WAIT_START;
                return 1U;
            }
            else
            {
                parser->state = PROTOCOL_PARSE_PAYLOAD;
            }
            break;

        case PROTOCOL_PARSE_PAYLOAD:
            parser->frame.payload[parser->index++] = byte;

            if (parser->index >= parser->frame.len)
            {
                *out = parser->frame;
                parser->state = PROTOCOL_PARSE_WAIT_START;
                return 1U;
            }
            break;

        default:
            protocol_parser_init(parser);
            break;
    }

    return 0U;
}

uint8_t protocol_encode_frame(app_msg_id_t message_id,
                              const uint8_t *payload,
                              uint8_t len,
                              uint8_t *out,
                              uint8_t out_max)
{
    if ((out == 0) ||
        (len > APP_PROTOCOL_PAYLOAD_MAX_LEN) ||
        (out_max < (uint8_t)(len + 3U)))
    {
        return 0U;
    }

    out[0] = PROTOCOL_START_BYTE;
    out[1] = (uint8_t)message_id;
    out[2] = len;

    if ((payload != 0) && (len > 0U))
    {
        memcpy(&out[3], payload, len);
    }

    return (uint8_t)(len + 3U);
}
