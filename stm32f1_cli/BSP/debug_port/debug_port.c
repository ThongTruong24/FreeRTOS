#include "debug_port.h"
#include "uart_port.h"

#define CONSOLE_UART_INSTANCE 2U
#define BSP_DEBUG_WRITE_TIMEOUT_MS 100U

void bsp_debug_init(void)
{
    /* UART hardware is initialized by the platform layer before app startup. */
}

void bsp_debug_write(const uint8_t *data, uint16_t len)
{
    (void)console_port_write(data, len, BSP_DEBUG_WRITE_TIMEOUT_MS);
}

uint8_t console_port_write(const uint8_t *data, uint16_t len, uint32_t timeout_ms)
{
    if ((data == 0) || (len == 0U) || (len > 255U)) {
        return 0;
    }

    return uart_port_write(CONSOLE_UART_INSTANCE, data, (uint8_t)len, timeout_ms);
}
