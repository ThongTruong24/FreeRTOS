#ifndef DEBUG_PORT_H
#define DEBUG_PORT_H

#include <stdint.h>

/* Application-facing debug BSP API. */
void bsp_debug_init(void);
void bsp_debug_write(const uint8_t *data, uint16_t len);

#endif /* DEBUG_PORT_H */
