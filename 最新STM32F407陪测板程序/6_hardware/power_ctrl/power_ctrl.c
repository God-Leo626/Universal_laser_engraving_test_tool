

#include "stm32f10x.h"
#include "power_ctrl.h"
/*******************************************************************
********************************************************************
* 名称：3.3v电源控制引脚初始化
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
*  PD6  3V3 (H : ON   L: OFF)
   PC12  5V (H : ON   L: OFF)
*********************************************************************
*********************************************************************/
void power_ctrl_init(void)
{
	GPIO_InitTypeDef    gpio_init_gpio ;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD,ENABLE);//使能时钟
	// PD6  推挽输出
	gpio_init_gpio.GPIO_Mode = GPIO_Mode_Out_PP;
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_6;
	gpio_init_gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&gpio_init_gpio);//初始化gpio口为推挽输出
	// PC12  推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOC,&gpio_init_gpio);//初始化gpio口为推挽输出
	// 将3v3电源关闭
	GPIO_ResetBits(GPIOD,GPIO_Pin_6); 
	//	将5v电源关闭
	GPIO_ResetBits(GPIOC,GPIO_Pin_12); 
}
/*******************************************************************
********************************************************************
* 名称：  void power_ctrl_ext(u8 pwr_select,bool on_off)
* 功能：   控制3v3 和 5v 的电源开关
* 参数：   on_off  1 ：打开     0： 关闭
* 返回值：  null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void power_ctrl_ext(u8 pwr_select,bool on_off)
{
	if(pwr_select == PWR_3V3_SELECT)
	{
		if(on_off == true)
		{
			POWER_3V3 = 1; // 3v3电源打开
		}
		else
		{
			POWER_3V3 = 0;  // 3v3电源关闭
		}
	
	}
	if(pwr_select == PWR_5V_SELECT)
	{
		if(on_off == true)
		{
			POWER_5V = 1; // 3v3电源打开
		}
		else
		{
			POWER_5V = 0;  // 3v3电源关闭
		}
	
	}
	

}




