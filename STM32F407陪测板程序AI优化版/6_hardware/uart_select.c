

#include "stm32f4xx.h"
#include "uart_select.h"
/*******************************************************************
********************************************************************
* 名称：串口选择初始化函数
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
*  PE6 : H : MCU 的串口1
         L : 自由串口 （可接测试模块）
   
*********************************************************************
*********************************************************************/
void uart_select_init(void)
{
	GPIO_InitTypeDef    gpio_init_gpio ;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIO E 时钟
	// PE6  推挽输出
	gpio_init_gpio.GPIO_Mode =  GPIO_Mode_OUT;     // 输出模式
	gpio_init_gpio.GPIO_OType =  GPIO_OType_PP;    // 推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_6;          // pin6
	gpio_init_gpio.GPIO_PuPd =  GPIO_PuPd_DOWN ;   // 下拉
	gpio_init_gpio.GPIO_Speed = GPIO_Speed_100MHz ;
	GPIO_Init(GPIOE,&gpio_init_gpio);//初始化gpio口为推挽输出

	//  将串口选择为stm32 的  uart1
	GPIO_SetBits(GPIOE,GPIO_Pin_6); 
}
/*******************************************************************
********************************************************************
* 名称： void uart_select(bool uart_select)
* 功能：   控制type c 串口接stm32 还是接DUT
* 参数：   uart_select  1 ：stm32     0： DUT
* 返回值：  null
* 版本：v1
* 作者：matt
* 日期：2025-06-11
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void uart_select(bool select)
{
	if(select == true)
	{
		GPIO_SetBits(GPIOE,GPIO_Pin_6); // GPOIO E 6  输出高电平，选择 stm32 uart1
	}
	else
	{
		GPIO_ResetBits(GPIOE,GPIO_Pin_6);  // GPOIO E 6  输出低电平，选择 DUT
	}
	

}




