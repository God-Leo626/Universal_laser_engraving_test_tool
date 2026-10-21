#include "stm32f4XX.h"
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
*  PB0 R    控制led亮灭（L:亮 H：灭）
*  
*********************************************************************
*********************************************************************/

void led_init()
{
	GPIO_InitTypeDef   gpio_init_strcut_gpio;

	//使能pb0   的时钟
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB ,ENABLE);
	
	// PB0 的io 初始化
	gpio_init_strcut_gpio.GPIO_Mode = GPIO_Mode_OUT; //普通输出模式
	gpio_init_strcut_gpio.GPIO_OType = GPIO_OType_PP; //推挽输出
	gpio_init_strcut_gpio.GPIO_Pin =  GPIO_Pin_0;
	gpio_init_strcut_gpio.GPIO_PuPd =GPIO_PuPd_UP; //上拉
	gpio_init_strcut_gpio.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	GPIO_Init(GPIOB, &gpio_init_strcut_gpio);  //初始化GPIO
	
	//PB0输出高电平 1  (灯灭)
	GPIO_SetBits(GPIOB, GPIO_Pin_0);


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
void led_ctrl(bool on_off)
{
	if(on_off == true)
	{
		LED = 0;  // led 亮
	}
	else
	{
		LED = 1;  // led 灭
	}
	
	
}



