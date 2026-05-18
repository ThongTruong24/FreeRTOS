#include "debug_port.h"

void bsp_debug_init(void)
{
    /* Console routing is selected by board_service. */
}

void bsp_debug_write(const uint8_t *data, uint16_t len)
{
    (void)data;
    (void)len;
}
