#ifndef GPIO_PORT_H
#define GPIO_PORT_H

#include "main.h"

void gpio_port_write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);
void gpio_port_toggle(GPIO_TypeDef *port, uint16_t pin);

#endif /* GPIO_PORT_H */
