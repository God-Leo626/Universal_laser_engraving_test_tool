#include "led.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "stdbool.h"

//=============================================
//  p0.04 引脚 控制led  H = 亮 ，L = 灭
void led_init(void)
{
//	nrf_gpio_range_cfg_output(17,20);   // 配置GPIO的引脚为输出
//	nrf_gpio_pin_set(LED_17);           // led 控制引脚输出高电平， led灭
//	nrf_gpio_pin_set(LED_18);           // led 控制引脚输出高电平， led灭
//	nrf_gpio_pin_set(LED_19);           // led 控制引脚输出高电平， led灭
//	nrf_gpio_pin_set(LED_20);           // led 控制引脚输出高电平， led灭
    
    nrf_gpio_cfg_output(4);                                    // 配置为输出
}

void led_ctrl(bool on,uint8_t pin_num)
{
	if(on == TRUE)
	{
		nrf_gpio_pin_clear(pin_num); // gpio 设置为0，led亮
	}
	else
	{
		nrf_gpio_pin_set(pin_num);  // gpio 设置为1，led灭
	}

}

void led_toggle(uint8_t pin_num)
{
	
	nrf_gpio_pin_toggle(pin_num);
	
}
