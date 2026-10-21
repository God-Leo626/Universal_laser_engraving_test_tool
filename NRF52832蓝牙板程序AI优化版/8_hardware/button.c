/** 开发板按键示例；当前扫描主流程不调用。 */
#include "button.h"
#include "nrf_gpio.h"
#include "buzzer.h"
#include "led.h"

void button_init(void)
{
    /* P0.13~P0.16：按下为低电平，松开为高电平。 */
    nrf_gpio_cfg_input(BUTTON_13, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BUTTON_14, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BUTTON_15, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BUTTON_16, NRF_GPIO_PIN_PULLUP);
}

void button_ctrl(uint8_t button_num, uint8_t led_num)
{
    if (nrf_gpio_pin_read(button_num) == 0) {
        /* 旧接口会等待松手；不要放进需持续消费广播的主循环。 */
        while (nrf_gpio_pin_read(button_num) == 0) { }
        buzzer_ctrl(true, 20);
        led_toggle(led_num);
    }
}
