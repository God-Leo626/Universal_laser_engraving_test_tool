#include "time7.h"
#include "lcd.h"
#include "esp8266.h"
#include <string.h>        // 使用字符串处理函数
#include <stdio.h>         //使用sprintf函数时包含
#include "variable_global.h"
#include "stm32f4xx.h"
#include "usart_dma_all.h"
#include "usart_ring_buff.h"
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"                               //任务头文件包含
//**************************************************************************************
/*******************************************************************
********************************************************************
* 名称：void TIM7_Int_Init(u16 arr,u16 psc)
* 功能：通用定时器中断初始化 
* 参数：arr：自动重装值 psc：时钟预分频数
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 通用定时器7中断初始化，这里时钟选择为APB1的2倍
	定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
* Ft=定时器工作频率,单位:Mhz 
*********************************************************************
*********************************************************************/
void time7_init(u16 arr,u16 psc)
{ 
	 NVIC_InitTypeDef NVIC_InitStructure;
	 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	 
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能    
	 
	 //定时器TIM7初始化
	 TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值 
	 TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	 TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	 
	 
	 NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=4 ;//抢占优先级0
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //子优先级2
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;   //IRQ通道使能
	 NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器
	 
	 TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	 TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE ); //使能指定的TIM7中断,允许更新中断
	 TIM_Cmd(TIM7,DISABLE);//关闭定时器7
 
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-12-24
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void time7_run(bool run)
{
	if(run == true)
	{
		TIM_Cmd(TIM7, ENABLE);       //使能 TIM7
	}
	else
	{
		TIM_Cmd(TIM7, DISABLE);       //使能 TIM7	
	}
	
}
/*******************************************************************
********************************************************************
* 名称：void TIM7_IRQHandler(void)
* 功能：//定时器7中断服务程序
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
*  TIM7_Int_Init(100-1,7200-1);  //TIM7 10ms触发中断
*********************************************************************
*********************************************************************/   
void TIM7_IRQHandler(void)
{  
	static s32 second_count = 16 ; //倒计时总起始时间
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET) //检查 TIM7 更新中断发生与否
	{
		second_count--; //总计数值减一
		if(second_count >= 0)
		{
			memset((void *)string_buf,0,strlen((const char*)string_buf));  // 清零string_buf
			sprintf((char *)string_buf,"%02d",second_count);
			lcd_fill_color(153,36,317,52,Tan);                  // 擦除 wifi名称显示位置
			display_ascii_string(153,36,8,(u8*)string_buf,Green,Tan);      // lcd 上显示时间倒计时
		
		}
		else
		{
			second_count = 16;
			usart1_printf("Please check wifi[ ssid: %s,password: %s ]\r\n",wifista_ssid,wifista_password);
			lcd_fill_color(153,54,317,70,Tan);                  // 擦除 password 显示位置
			display_ascii_string(153,54,8,(u8*)"PLEASE CHECK WIFI",Red,Tan);               // lcd 上显示检查wifi热点是否存在	
		}

		TIM_ClearITPendingBit(TIM7, TIM_IT_Update ); //清除 TIM7 更新中断标志
	}
	
}




