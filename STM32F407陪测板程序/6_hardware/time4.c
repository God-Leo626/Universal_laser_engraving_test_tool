#include "time4.h"
#include "stm32f10x.h"
#include "stdbool.h"
#include "button.h"
//===========================================================================

u32 time4_count2 =0;  // 用于测试计数
/*******************************************************************
********************************************************************
* 名称：
* 功能：time4 通用定时器初始化
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 16bit 计数器
* APB1 的时钟为 2 分频 APB1 的时钟为 36M 当 APB1 的时钟分频数为 1 的
时候， TIM2~7 的时钟为 APB1 的时钟
*********************************************************************
*********************************************************************/
void time4_init(u16 reload,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitTypeDef_time4;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);  // 开启定时器的时钟
	
	TIM_TimeBaseInitTypeDef_time4.TIM_ClockDivision     = TIM_CKD_DIV1;        // 设置时钟分割
	TIM_TimeBaseInitTypeDef_time4.TIM_CounterMode       = TIM_CounterMode_Up;  // TIM 向上计数
	TIM_TimeBaseInitTypeDef_time4.TIM_Period            = reload;                 // 设置自动重装载寄存器周期的值
	TIM_TimeBaseInitTypeDef_time4.TIM_Prescaler         = psc;                 // 设置时钟频率除数的预分频值
	TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitTypeDef_time4);
	
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE );                 //允许更新中断
	//中断优先级 NVIC 设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;           //TIM4 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //抢占优先级 0 级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        //子优先级 3 级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           //IRQ 通道被使能
	NVIC_Init(&NVIC_InitStructure);                           //初始化 NVIC 寄存器
	
//	TIM_Cmd(TIM4, ENABLE);                                    //使能 TIM4
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
void time4_run(bool run)
{
	if(run == true)
	{
		TIM_Cmd(TIM4, ENABLE);       //使能 TIM4
	}
	else
	{
		TIM_Cmd(TIM4, DISABLE);       //使能 TIM4	
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

void TIM4_IRQHandler(void) //TIM3 中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) //检查 TIM4 更新中断发生与否
	{
		if(SW1 == 0)   // sw1 按键持续保持按住状态才计数
		{
				time4_count2++;
		}
		
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update ); //清除 TIM4 更新中断标志
	}
}





