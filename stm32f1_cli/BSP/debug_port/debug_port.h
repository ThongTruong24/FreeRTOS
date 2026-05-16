#ifndef DEBUG_PORT_H
#define DEBUG_PORT_H

#include <stdint.h>

/* Application-facing debug BSP API. */
void bsp_debug_init(void);
void bsp_debug_write(const uint8_t *data, uint16_t len);

/* Legacy low-level console API kept for existing users. */
uint8_t console_port_write(const uint8_t *data, uint16_t len, uint32_t timeout_ms);

#endif /* DEBUG_PORT_H */
