/** 开发板触摸键示例；当前扫描主流程不调用。 */
#include "touch_pad.h"
#include "nrf_gpio.h"
#include "buzzer.h"
#include "led.h"

void touch_pad_init(void)
{
    /* P0.03，沿用原来的上拉配置；实际触摸模块有效电平以硬件为准。 */
    nrf_gpio_cfg_input(TOUCH_BUTTON_03, NRF_GPIO_PIN_PULLUP);
}

void touch_ctrl(void)
{
    if (nrf_gpio_pin_read(TOUCH_BUTTON_03) == 1) {
        /* 保留等待释放的旧行为；扫描主流程不应调用此阻塞示例。 */
        while (nrf_gpio_pin_read(TOUCH_BUTTON_03) == 1) { }
        buzzer_ctrl(true, 20);
        led_toggle(LED_17);
        led_toggle(LED_18);
        led_toggle(LED_19);
        led_toggle(LED_20);
    }
}
