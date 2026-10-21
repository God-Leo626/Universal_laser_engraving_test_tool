#include "button.h"
#include "stm32f10x.h"
#include "systick_delay_freertos.h"
#include "buzzer.h"
#include "stdbool.h"
#include "my_task.h"
#include "lcd.h"

#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"
#include "queue.h"                             // 使用消息队列要包含的头文件
#include "semphr.h"                            //信号量头文件包含
//======================================================================================
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
* PB15 :KEY1  (L:按下   H :没有按下)
  PB14: KEY2  (L:按下   H :没有按下)
  PC7  :KEY3  (L:按下   H :没有按下)
*********************************************************************
*********************************************************************/
void button_pin_init(void)
{
	//初始化GPIO 的结构体变量
	GPIO_InitTypeDef  GPIO_TypeDef_init;
	//外部中断线的结构体变量，用来初始化外部中断
	EXTI_InitTypeDef  exti_init_type_strcut;
	//初始化中断优先级的结构体变量
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC,ENABLE);  // 使能gpio B C口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);   //使能外设中断时钟
	//初始化 pin的输入模式，速度，pin选择
	// PB15
	GPIO_TypeDef_init.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_15;
	GPIO_TypeDef_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_TypeDef_init);
	// PB14
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_TypeDef_init);
	// PC7
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOC,&GPIO_TypeDef_init);
	
	//外部中断线的映射关系 PB14的中断
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource14);
	//外部中断线的映射关系 PB15的中断
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource15);
	//外部中断线的映射关系 PC7的中断
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource7);
	
	
	//初始化外部中断 PB15
	exti_init_type_strcut.EXTI_Line = EXTI_Line15;                 //选定 15线
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;                   //使能线中断
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;         //选择中断模式
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Falling;     //下降沿触发中断
	EXTI_Init(&exti_init_type_strcut);
	
	
	//初始化外部中断 PB14
	exti_init_type_strcut.EXTI_Line = EXTI_Line14;                 //选定 14线
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;                   //使能线中断
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;         //选择中断模式
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Falling;     //下降沿触发中断
	EXTI_Init(&exti_init_type_strcut);
	
	//初始化外部中断PC7
	exti_init_type_strcut.EXTI_Line = EXTI_Line7;
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&exti_init_type_strcut);
	
	
	//配置中断分组 PB15
	//配置中断分组 PB14
	nvic_init_type_strcut.NVIC_IRQChannel = EXTI15_10_IRQn;
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 7;
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic_init_type_strcut);
	
	
	
	//配置中断分组 PC7
	nvic_init_type_strcut.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 9;
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;
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
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void  EXTI15_10_IRQHandler(void)
{
	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
	ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
	buzzer_ctrl(3,true);
	if(EXTI_GetITStatus(EXTI_Line15) == SET) // 监测是那个中断线产生的中断
	{
		printf("key1\r\n");	
		EXTI_ClearITPendingBit(EXTI_Line15);//清除中断标志d
	}
	else if(EXTI_GetITStatus(EXTI_Line14) == SET)
	{
		worknumber_config_global_flag = !worknumber_config_global_flag ;
		if(worknumber_config_global_flag == true)
		{
			usart1_printf("WORKNUMBER CONFIG MODE\r\n");	
			display_ascii_string(64,0,16,(u8*)"SERVER    DL:",Green,Tan);          // 显示工单配置提示
			vTaskResume(WORKNUMBER_CONFIG_TASK_Task_Handler); // 解挂
		}
		else
		{
			usart1_printf("COOLKIT TEST MODE\r\n");	
			display_ascii_string(64,0,16,(u8*)"COOLKIT TEST:",Blue,Tan);          // 显示测试模式字符
			vTaskSuspend(WORKNUMBER_CONFIG_TASK_Task_Handler);            // 挂起
		}
		
		EXTI_ClearITPendingBit(EXTI_Line14);//清除中断标志
	}
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );  // 退出临界代码段保护

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
void  EXTI9_5_IRQHandler(void)
{
	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
//	BaseType_t    xsemaphore_back ; //保存二值信号量give 函数的返回值
	BaseType_t    pxHigherPriorityTaskWoken = pdFALSE ;
	//**************************************************************************************
	ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
	
	buzzer_ctrl(3,true);
	if(EXTI_GetITStatus(EXTI_Line7) == SET)
	{
//		usart1_printf("key3\r\n");
//		xsemaphore_back = xSemaphoreGiveFromISR(esp8266_online_semaphore_handle,&pxHigherPriorityTaskWoken);
//		if(xsemaphore_back != pdTRUE)
//		{
//			usart1_printf("%s\r\n","[** key3 interrupt **] semaphore full,semaphore give[== fail ==]");
//		}
		test_global_flag = !test_global_flag ; // 表示纯测试模式（true 不激活，false 全功能模式），按一次功能对调一次
		usart1_printf("test_global_flag = %d\r\n",test_global_flag);
	}
	EXTI_ClearITPendingBit(EXTI_Line7);//清除中断标志
	// 判断是否在出中断的时候要进行任务切换
	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );  // 退出临界代码段保护

}




