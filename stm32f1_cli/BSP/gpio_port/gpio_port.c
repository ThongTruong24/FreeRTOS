#include "gpio_port.h"
#include "main.h"

#define LED_GPIO_PORT GPIOC
#define LED_GPIO_PIN GPIO_PIN_13
#define LED_ACTIVE_LOW 1
#define BSP_LED_STATUS_ID 0U

static uint8_t gpio_port_led_id_is_valid(uint8_t led_id);

void bsp_led_init(void)
{
    gpio_port_init();
}

void bsp_led_on(uint8_t led_id)
{
    if (gpio_port_led_id_is_valid(led_id) != 0U) {
        gpio_port_on();
    }
}

void bsp_led_off(uint8_t led_id)
{
    if (gpio_port_led_id_is_valid(led_id) != 0U) {
        gpio_port_off();
    }
}

void bsp_led_toggle(uint8_t led_id)
{
    if (gpio_port_led_id_is_valid(led_id) != 0U) {
        gpio_port_toggle();
    }
}

void gpio_port_init(void)
{
    gpio_port_off();
}

void gpio_port_on(void)
{
#if LED_ACTIVE_LOW
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
#else
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
#endif
}

void gpio_port_off(void)
{
#if LED_ACTIVE_LOW
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_SET);
#else
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GPIO_PIN, GPIO_PIN_RESET);
#endif
}

void gpio_port_toggle(void)
{
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_GPIO_PIN);
}

void gpio_port_write(uint8_t enabled)
{
    if (enabled != 0U) {
        gpio_port_on();
    } else {
        gpio_port_off();
    }
}

static uint8_t gpio_port_led_id_is_valid(uint8_t led_id)
{
    return (uint8_t)(led_id == BSP_LED_STATUS_ID);
}
