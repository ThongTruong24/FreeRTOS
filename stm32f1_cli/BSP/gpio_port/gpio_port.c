#include "gpio_port.h"

void gpio_port_write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state)
{
    if (port == 0)
    {
        return;
    }

    HAL_GPIO_WritePin(port, pin, state);
}

void gpio_port_toggle(GPIO_TypeDef *port, uint16_t pin)
{
    if (port == 0)
    {
        return;
    }

    HAL_GPIO_TogglePin(port, pin);
}
