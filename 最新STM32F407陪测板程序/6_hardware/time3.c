#include "time3.h"
#include "stm32f4xx.h"
#include "stdbool.h"
#include "lcd.h"
#include "stdio.h"
#include "variable_global.h"
#include "string.h"
//===========================================================================
bool pc_reback_msg_to_usart1_timeout  =  false ;   // pc返回给串口1的消息超时未返回

/*******************************************************************
********************************************************************
* 名称：
* 功能：time3 通用定时器初始化
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 16bit 计数器
* APB1 的时钟为 2 分频 APB1 的时钟为 84M 当 APB1 的时钟分频数为 1 的
时候， TIM2~7 的时钟为 APB1 的时钟
*********************************************************************
*********************************************************************/
void time3_init(u16 reload,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeDef_time3;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);  // 开启定时器的时钟
	
	TIM_TimeBaseInitTypeDef_time3.TIM_ClockDivision     = TIM_CKD_DIV1;        // 设置时钟分割
	TIM_TimeBaseInitTypeDef_time3.TIM_CounterMode       = TIM_CounterMode_Up;  // TIM 向上计数
	TIM_TimeBaseInitTypeDef_time3.TIM_Period            = reload;                 // 设置自动重装载寄存器周期的值
	TIM_TimeBaseInitTypeDef_time3.TIM_Prescaler         = psc;                 // 设置时钟频率除数的预分频值
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitTypeDef_time3);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE );                 //允许更新中断
	//中断优先级 NVIC 设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;           //TIM3 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4; //抢占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;        //子优先级 3 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);                           //初始化 NVIC 寄存器
	
//	TIM_Cmd(TIM3, ENABLE);                                    //使能 TIM3
}
/*******************************************************************
********************************************************************
* 名称：void time3_run(bool run)
* 功能：开启 关闭 time3
* 参数： run： 1 开    0  关
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void time3_run(bool run)
{
	if(run == true)
	{
		TIM_Cmd(TIM3, ENABLE);       //使能 TIM3
	}
	else
	{
		TIM_Cmd(TIM3, DISABLE);       //使能 TIM3	
	}
	
}

/*******************************************************************
********************************************************************
* 名称：void TIM3_IRQHandler(void)
* 功能：//定时器 3 中断服务程序
* 参数：null
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

void TIM3_IRQHandler(void) //TIM3 中断
{
	u32 h = 0,m = 0,s = 0; // 定义时分秒的变量
	static bool display_flag = false ;
	static char time_string_buf[10] = {0};
	u32 second_count = 0 ;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查 TIM3 更新中断发生与否
	{
		time3_count1++;
		if(time3_count1 % 2 == 0)  //  1s 显示时间刷新一次
		{
			display_flag = ! display_flag ;
			second_count = time3_count1*500/1000 ;     //总秒数
			h = second_count / 3600 ;                  //时
			m = (second_count % 3600 )/60;             //分
			s = (second_count % 3600 )%60;  ;          // 秒

			//==================显示小时=========================================================
			memset((void*)time_string_buf,0,sizeof(time_string_buf)); // 清空string_buf,不	清空会导致显示乱码
			sprintf((char *)time_string_buf,"[%02u:%02u:%02u]",h,m,s);
			if(display_flag)
			{
				lcd_fill_color(210,440,318,477,Black);  // 清屏上一次测试显示的时间，为接下来的新时间显示做准备
				display_ascii_string(224,450,8,(u8*)time_string_buf,Purple,Black);	
			}
			else
			{
				lcd_fill_color(210,440,318,477,Black);  // 清屏上一次测试显示的时间，为接下来的新时间显示做准备
				display_ascii_string(224,450,8,(u8*)time_string_buf,OrangeRed,Black);	
			}
		}
        if(time3_count1 == 10) //  表示5s时间到
        {
            pc_reback_msg_to_usart1_timeout = true ;  // 超时，标志置1
        }
        
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update ); //清除 TIM3 更新中断标志
	}
}





