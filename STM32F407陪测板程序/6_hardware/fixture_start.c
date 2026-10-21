#include "stm32f4xx.h"
#include "fixture_start.h"
#include "time3.h"
//#include "time4.h"
#include "systick_delay_freertos.h"
#include "io_bit.h"
#include "stdbool.h"
#include "lcd.h"
//#include "test_logic.h"
#include "variable_global.h"
#include "stdio.h"
#include "power_ctrl.h"
#include "buzzer.h"
#include "FreeRTOS.h"                          // 使用freertos 操作系统时候包含头文件
#include "queue.h"                             // 使用消息队列要包含的头文件
#include "semphr.h"                            //信号量头文件包含
#include "my_task.h"
#include "ads1262.h"
#include "button.h"
#include "relay.h"
#include "max49918.h"
//===========================================================================
//bool fixture_status_flag = false; // 治具状态保存  false ： 治具打开  true： 下压到位


/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* PE0
*********************************************************************
*********************************************************************/
void fixture_start_exti_init(void)
{
	//初始化GPIO 的结构体变量
	GPIO_InitTypeDef  GPIO_TypeDef_init;
	//外部中断线的结构体变量，用来初始化外部中断
	EXTI_InitTypeDef  exti_init_type_strcut;
	//初始化中断优先级的结构体变量
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);   // 使能gpio E口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);   //使能外设中断时钟
	//初始化 pin的输入模式，速度，pin选择
	GPIO_TypeDef_init.GPIO_Mode =  GPIO_Mode_IN;
//	GPIO_TypeDef_init.GPIO_OType = GPIO_OType_PP;
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_TypeDef_init.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_TypeDef_init.GPIO_Speed = GPIO_Speed_100MHz ;
	GPIO_Init(GPIOE,&GPIO_TypeDef_init);
	
	


	//外部中断线的映射关系 PC4的中断
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);   //使能外设中断时钟
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);//PE0 连接到中断线0
	//初始化外部中断 PC4
	exti_init_type_strcut.EXTI_Line = EXTI_Line0;                         //选定 0线                   
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;                          //使能线中断
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;                //选择中断模式
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Rising_Falling;     //下降沿,上升沿均能触发中断
	EXTI_Init(&exti_init_type_strcut);
	
	
	//配置中断分组 PC4
	nvic_init_type_strcut.NVIC_IRQChannel = EXTI0_IRQn ;             // 中断线选择
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 5;    // 抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;           // 子优先级，使用功能freeRTOS时，子优先级设置为0
	NVIC_Init(&nvic_init_type_strcut);
	
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
* PE0
*********************************************************************
*********************************************************************/
void fixture_start_init_for_calibration(void)
{
	//初始化GPIO 的结构体变量
	GPIO_InitTypeDef  GPIO_TypeDef_init;
	//外部中断线的结构体变量，用来初始化外部中断
	// EXTI_InitTypeDef  exti_init_type_strcut;
	//初始化中断优先级的结构体变量
	// NVIC_InitTypeDef  nvic_init_type_strcut;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);   // 使能gpio E口时钟
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);   //使能外设中断时钟
	//初始化 pin的输入模式，速度，pin选择
	GPIO_TypeDef_init.GPIO_Mode =  GPIO_Mode_IN;   //普通输入模式
//	GPIO_TypeDef_init.GPIO_OType = GPIO_OType_PP;   
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_TypeDef_init.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_TypeDef_init.GPIO_Speed = GPIO_Speed_100MHz ;
	GPIO_Init(GPIOE,&GPIO_TypeDef_init);
}


/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* PE0
* 
*********************************************************************
*********************************************************************/
void EXTI0_IRQHandler(void)
{ 
	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
	BaseType_t  pxHigherPriorityTaskWoken = pdFALSE ;
//	BaseType_t  start_queue_send_back ;
//	u8 fixture_down_meg = 0;     // 治具下压时，保存要发送的消息队列值
//	u8 fixture_up_meg = 1;       // 治具抬起时，保存要发送的消息队列值
	//**************************************************************************************
	ulReturn = taskENTER_CRITICAL_FROM_ISR();                          // 进入临界代码段
	systick_delay_ms(15);
	
	if(START == 0) //治具按下 为低电平，发送队列消息0
	{

	}
	else  //治具上抬，为高电平，发送队列消息1
	{
		
	}
	EXTI_ClearITPendingBit(EXTI_Line0);                                //清除中断标志
	// 判断是否在出中断的时候要进行任务切换
	if(pxHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
	}
	
	//**************************************************************************************
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护
	
}



