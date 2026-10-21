


#include "stm32f10x.h"
#include "led.h"
#include "stdbool.h"

/*******************************************************************
********************************************************************
* 名称： led初始化函数
* 功能： 初始化lcd pin
* 参数：  null
* 返回值： null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
*  PB13 R    控制led亮灭（L:亮 H：灭）
*  PC6  G    控制led亮灭（L:亮 H：灭）
   PC8  B    控制led亮灭（L:亮 H：灭）
*********************************************************************
*********************************************************************/

void led_init()
{
	GPIO_InitTypeDef   gpio_init_strcut_gpio;

	
	//使能pb13  pc6  pc8    的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC , ENABLE);

	
	// PB4 的io 初始化
	gpio_init_strcut_gpio.GPIO_Pin   = GPIO_Pin_13;
	gpio_init_strcut_gpio.GPIO_Speed = GPIO_Speed_50MHz; 
	gpio_init_strcut_gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &gpio_init_strcut_gpio);
	
	// PC 6
	gpio_init_strcut_gpio.GPIO_Pin   = GPIO_Pin_6;
	GPIO_Init(GPIOC, &gpio_init_strcut_gpio);
	
	// PC 8
	gpio_init_strcut_gpio.GPIO_Pin   = GPIO_Pin_8;
	GPIO_Init(GPIOC, &gpio_init_strcut_gpio);


	
	//PB13  PC6  ,PC8输出高电平 1  (灯灭)
	GPIO_SetBits(GPIOB, GPIO_Pin_13);
	GPIO_SetBits(GPIOC, GPIO_Pin_6);
	GPIO_SetBits(GPIOC, GPIO_Pin_8);


}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void rgb_led_ctrl(bool r,bool g, bool b)
{
	if(r == true)
	{
		LED_R = 0;  // led 红色亮
	}
	else
	{
		LED_R = 1;  // led 红色灭
	}
	if(g == true)
	{
		LED_G = 0;  //led 绿色 亮
	}
	else
	{
		LED_G = 1;  //led 绿色 灭
	}
	if(b == true)
	{
		LED_B = 0;  //led 蓝色 亮
	}
	else
	{
		LED_B = 1;   //led 蓝色 灭
	}
	
}



