#include "button.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "buzzer.h"
#include "led.h"


void button_init(void)
{
	// 板端有4个物理按键， 按键按下后为低，松开后为高
	// 按键的 gpio口为 p0.13   p0.14   p0.15   p0.16 
	
	nrf_gpio_cfg_input(BUTTON_13,NRF_GPIO_PIN_PULLUP);  //  配置引脚13 为上拉输入
	nrf_gpio_cfg_input(BUTTON_14,NRF_GPIO_PIN_PULLUP);  //  配置引脚14 为上拉输入
	nrf_gpio_cfg_input(BUTTON_15,NRF_GPIO_PIN_PULLUP);  //  配置引脚15 为上拉输入
	nrf_gpio_cfg_input(BUTTON_16,NRF_GPIO_PIN_PULLUP);  //  配置引脚16 为上拉输入

}

void button_ctrl(uint8_t button_num,uint8_t led_num)
{
	if(nrf_gpio_pin_read(button_num) == 0)
	{
		while(nrf_gpio_pin_read(button_num) == 0);
		
		buzzer_ctrl(true, 20) ;
		led_toggle(led_num) ;
	}

}
