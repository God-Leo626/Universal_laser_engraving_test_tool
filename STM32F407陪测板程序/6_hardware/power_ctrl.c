

#include "stm32f4xx.h"
#include "power_ctrl.h"
/*******************************************************************
********************************************************************
* 名称：3.3v/ 5v被测模块，esp8266模块电源控制引脚初始化
* 功能： 初始化nrf52832的电源控制引脚
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
*  PE3  3V3 (H : ON   L: OFF)   nrf52832的电源控制初始化
   
   
*********************************************************************
*********************************************************************/
void power_ctrl_init(void)
{
	GPIO_InitTypeDef    gpio_init_gpio ;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOC  时钟
	// Pc11  推挽输出
	gpio_init_gpio.GPIO_Mode =  GPIO_Mode_OUT;     // 输出模式
	gpio_init_gpio.GPIO_OType =  GPIO_OType_PP;    // 推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_11;          // pin11
	gpio_init_gpio.GPIO_PuPd =  GPIO_PuPd_DOWN ;   // 下拉
	gpio_init_gpio.GPIO_Speed = GPIO_Speed_100MHz ;
	GPIO_Init(GPIOE,&gpio_init_gpio);//初始化gpio口为推挽输出
	// PC12  推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOE,&gpio_init_gpio);//初始化gpio口为推挽输出

	
	//  将3v3电源关闭
	GPIO_ResetBits(GPIOE,GPIO_Pin_3); 
	
	
}
/*******************************************************************
********************************************************************
* 名称：  void power_ctrl_ext(bool on_off)
* 功能：   控制3v3 的电源开关
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
void power_ctrl_nrf52832(bool on_off)
{
	if(on_off)
	{
        POWER_3V3 = 1 ;  // 打开nrf52832的电源
	}
    else
    {
        POWER_3V3 = 0 ; // 关闭52832的电源
    }


}




