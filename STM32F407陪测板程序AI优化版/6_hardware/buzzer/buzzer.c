#include  "buzzer.h"
#include  "stm32f4xx.h" 
#include  "stdbool.h"
#include  "systick_delay_freertos.h"

//======================================================================================
/*******************************************************************
********************************************************************
* 名称：void buzzer_init(void)
* 功能： 初始化蜂鸣器控制引脚
* 参数： 无
* 返回值： 无
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* Pc10  (H: 鸣叫  L ： 关闭)
*********************************************************************
*********************************************************************/
void buzzer_init(void)
{
	GPIO_InitTypeDef    gpio_init_gpio ;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOE时钟
	// PE2  推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_10;
	gpio_init_gpio.GPIO_Mode = GPIO_Mode_OUT;  //普通输出模式
	gpio_init_gpio.GPIO_OType = GPIO_OType_PP;//推挽输出
	gpio_init_gpio.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	gpio_init_gpio.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
	GPIO_Init(GPIOC,&gpio_init_gpio);//初始化gpio口为推挽输出

	
	//	将蜂鸣器关闭
	GPIO_ResetBits(GPIOE,GPIO_Pin_10); 
}
/*******************************************************************
********************************************************************
* 名称：void buzzer_ctrl(u16 runtime , bool on_off)
* 功能：控制蜂鸣器鸣叫制定的时间后关闭
* 参数：u16 runtime:鸣叫的时间 , bool on_off： true： on  false ：off
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void buzzer_ctrl(u16 runtime , bool on_off)
{
	if(on_off == true)
	{
		BUZZER_CTRL = 1 ;
		systick_delay_ms(runtime);
		BUZZER_CTRL = 0 ;  // 不鸣叫
	}
	else
	{
		BUZZER_CTRL = 0 ;  // 不鸣叫
	}

}

