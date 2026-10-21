#include "buzzer.h"
#include "stdint.h"
#include "stdbool.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

//*****************************************************************************************************************
void buzzer_init(void)
{
	nrf_gpio_cfg_output(BUZZER);        // 配置GPIO的引脚为输出
	nrf_gpio_pin_clear(BUZZER);           // GPIO 输出低电平 蜂鸣器  不鸣叫
}



void buzzer_ctrl(bool on_off, uint16_t time)
{
	if(on_off == TRUE)
	{
		nrf_gpio_pin_set(BUZZER);  // 输入高电平， 蜂鸣器鸣叫
		if(time <= 65535)
		{
			nrf_delay_ms(time);
			nrf_gpio_pin_clear(BUZZER);  // 输出低电平， 蜂鸣器不叫
		}
		else
		{
			nrf_gpio_pin_clear(BUZZER);  // 输出低电平， 蜂鸣器不叫
		}
	}
	else
	{
		nrf_gpio_pin_clear(BUZZER);  // 输出低电平， 蜂鸣器不叫
	}

}



