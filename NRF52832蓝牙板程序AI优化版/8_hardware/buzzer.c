/** 保留的蜂鸣器接口；当前扫描主流程不调用。 */
#include "buzzer.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

void buzzer_init(void)
{
    nrf_gpio_cfg_output(BUZZER);
    nrf_gpio_pin_clear(BUZZER);
}

void buzzer_ctrl(bool on_off, uint16_t time)
{
    if (on_off) {
        nrf_gpio_pin_set(BUZZER);
        /* time 为 uint16_t，无需再判断是否超过 65535。此接口阻塞 time ms。 */
        nrf_delay_ms(time);
    }
    nrf_gpio_pin_clear(BUZZER);
}
