#include "calibration.h"
#include "stm32f4xx.h"
#include "systick_delay_freertos.h"
/*====================================================================*/

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2025-02-16
* 举例：
* 备注：
* 
* 
CAL1---  NO1  ---  PC6  --- 10M
CAL2---  NO2  ---  PC7  --- 1M
CAL3---  NO3  ---  PC8  ---100K
CAL4---  NO4  ---  PC9  ---10K

CAL5---        ---  PA8  ---1K --- H

CAL6---        ---  PA11  ---100R--- H

CAL7---        ---  PA12  --- 20R--- H
*********************************************************************
*********************************************************************/
void current_calibration_init()
{
	GPIO_InitTypeDef    gpio_init_gpio ;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOA GPIOc时钟
	// PA8  , PA11 , PA12  推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_11|GPIO_Pin_12;
	gpio_init_gpio.GPIO_Mode = GPIO_Mode_OUT;  //普通输出模式
	gpio_init_gpio.GPIO_OType = GPIO_OType_PP;//推挽输出
	gpio_init_gpio.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	gpio_init_gpio.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
	GPIO_Init(GPIOA,&gpio_init_gpio);//初始化gpio口为推挽输出

	// PC 6 7 8 9
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
	GPIO_Init(GPIOC,&gpio_init_gpio);//初始化gpio口为推挽输出
	//	将gpio口均初始化为低
	GPIO_ResetBits(GPIOA,GPIO_Pin_8|GPIO_Pin_11|GPIO_Pin_12); 
	GPIO_ResetBits(GPIOC,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9); 


}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2025-02-16
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

void calibration_ctrl(u8 resistor_select)
{
	if(resistor_select == R10M)
	{
		CTRL_PC6 = 1 ;
		CTRL_PC7 = 0;
		CTRL_PC8 = 0;
		CTRL_PC9 = 0;
		CTRL_PA8 = 0;
		CTRL_PA11 = 0;
		CTRL_PA12 = 0;
		systick_delay_ms(1);
	}
	else if(resistor_select == R1M)
	{
		CTRL_PC6 = 0 ;
		CTRL_PC7 = 1;
		CTRL_PC8 = 0;
		CTRL_PC9 = 0;
		CTRL_PA8 = 0;
		CTRL_PA11 = 0;
		CTRL_PA12 = 0;
		systick_delay_ms(1);
	}
	else if(resistor_select == R100K)
	{
		CTRL_PC6 = 0 ;
		CTRL_PC7 = 0;
		CTRL_PC8 = 1;
		CTRL_PC9 = 0;
		CTRL_PA8 = 0;
		CTRL_PA11 = 0;
		CTRL_PA12 = 0;
		systick_delay_ms(1);
	}
	else if(resistor_select == R10K)
	{
		CTRL_PC6 = 0 ;
		CTRL_PC7 = 0;
		CTRL_PC8 = 0;
		CTRL_PC9 = 1;
		CTRL_PA8 = 0;
		CTRL_PA11 = 0;
		CTRL_PA12 = 0;
		systick_delay_ms(1);
	}
	else if(resistor_select == R1K)
	{
		CTRL_PC6 = 0 ;
		CTRL_PC7 = 0;
		CTRL_PC8 = 0;
		CTRL_PC9 = 0;
		CTRL_PA8 = 1;
		CTRL_PA11 = 0;
		CTRL_PA12 = 0;
		systick_delay_ms(1);
	}
	else if(resistor_select == R100)
	{
		CTRL_PC6 = 0 ;
		CTRL_PC7 = 0;
		CTRL_PC8 = 0;
		CTRL_PC9 = 0;
		CTRL_PA8 = 0;
		CTRL_PA11 = 1;
		CTRL_PA12 = 0;
		systick_delay_ms(1);
	}
	else if(resistor_select == R20)
	{
		CTRL_PC6 = 0 ;
		CTRL_PC7 = 0;
		CTRL_PC8 = 0;
		CTRL_PC9 = 0;
		CTRL_PA8 = 0;
		CTRL_PA11 = 0;
		CTRL_PA12 = 1;
		systick_delay_ms(1);
	}


}