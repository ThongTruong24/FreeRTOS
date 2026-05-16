#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include <stdint.h>
#include "message.h"

uint8_t board_config_apply(const app_board_config_msg_t *msg);
const char *board_config_bus_name(board_bus_t bus);

#endif
