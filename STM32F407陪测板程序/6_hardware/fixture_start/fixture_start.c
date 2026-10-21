#include "stm32f10x.h"
#include "fixture_start.h"
#include "time3.h"
#include "time4.h"
#include "systick_delay.h"
#include "io_bit.h"
#include "stdbool.h"
#include "lcd.h"
#include "test_logic.h"
#include "systick_delay.h"
#include "stdio.h"
#include "power_ctrl.h"
#include "buzzer.h"
#include "time3.h"
#include "FreeRTOS.h"                          // 使用freertos 操作系统时候包含头文件
#include "queue.h"                             // 使用消息队列要包含的头文件
#include "semphr.h"                            //信号量头文件包含
#include "my_task.h"
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
* PB1
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
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);  // 使能gpio B口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);   //使能外设中断时钟
	//初始化 pin的输入模式，速度，pin选择
	GPIO_TypeDef_init.GPIO_Mode = GPIO_Mode_IPU;          // 上拉输入
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_1;
	GPIO_TypeDef_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_TypeDef_init);

	//外部中断线的映射关系 PB1的中断
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource1);
	
	//初始化外部中断 PB1
	exti_init_type_strcut.EXTI_Line = EXTI_Line1;                         //选定 1线                   
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;                          //使能线中断
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;                //选择中断模式
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Rising_Falling;     //下降沿,上升沿均能触发中断
	EXTI_Init(&exti_init_type_strcut);
	
	
	//配置中断分组 PB1
	nvic_init_type_strcut.NVIC_IRQChannel = EXTI1_IRQn;             // 中断线选择
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
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void EXTI1_IRQHandler(void)
{ 
	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
	BaseType_t  pxHigherPriorityTaskWoken = pdFALSE ;
	BaseType_t  start_queue_send_back ;
	u8 fixture_down_meg = 0;     // 治具下压时，保存要发送的消息队列值
	u8 fixture_up_meg = 1;     // 治具下压时，保存要发送的消息队列值
	//**************************************************************************************
	ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
	buzzer_ctrl(5,true);
	if(START == 0) //治具按下 为低电平，发送队列消息0
	{
		start_queue_send_back =  xQueueSendToBackFromISR(fixture_start_queue_handle,(void*)&fixture_down_meg,&pxHigherPriorityTaskWoken);
		if(start_queue_send_back == pdTRUE)
		{
//			printf("[** exti1 interrupt **]fixture [down],fixture_start_queue_send 0 [ == ok == ]\r\n");
		}
	}
	else  //治具上抬，为高电平，发送队列消息1
	{
		start_queue_send_back =  xQueueSendToBackFromISR(fixture_start_queue_handle,(void*)&fixture_up_meg,&pxHigherPriorityTaskWoken);
		if(start_queue_send_back == pdTRUE)
		{
//			printf("[** exti1 interrupt **]fixture [up],fixture_start_queue_send 1 [ == ok == ]\r\n");
		}
	}
	// 判断是否在出中断的时候要进行任务切换
	if(pxHigherPriorityTaskWoken == pdTRUE)
	{
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
	}
	EXTI_ClearITPendingBit(EXTI_Line1);                                //清除中断标志
	//**************************************************************************************
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护
	
}



