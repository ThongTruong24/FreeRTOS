#ifndef GPIO_PORT_H
#define GPIO_PORT_H

#include <stdint.h>

/* Application-facing LED BSP API. */
void bsp_led_init(void);
void bsp_led_on(uint8_t led_id);
void bsp_led_off(uint8_t led_id);
void bsp_led_toggle(uint8_t led_id);

/* Legacy low-level GPIO port API kept for existing users. */
void gpio_port_init(void);
void gpio_port_on(void);
void gpio_port_off(void);
void gpio_port_toggle(void);
void gpio_port_write(uint8_t enabled);

#endif /* GPIO_PORT_H */
