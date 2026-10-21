/** 当前主循环仅使用 P0.04 翻转作为运行指示。 */
#include "led.h"
#include "nrf_gpio.h"

void led_init(void)
{
    nrf_gpio_cfg_output(LED_04);
}

void led_ctrl(bool on, uint8_t pin_num)
{
    /* 保留旧接口的低电平点亮约定。P0.04 实际极性以原理图为准。 */
    if (on) {
        nrf_gpio_pin_clear(pin_num);
    } else {
        nrf_gpio_pin_set(pin_num);
    }
}

void led_toggle(uint8_t pin_num)
{
    nrf_gpio_pin_toggle(pin_num);
}
