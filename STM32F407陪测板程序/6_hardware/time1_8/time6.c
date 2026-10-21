#include "time6.h"
#include "my_task.h"
#include "stm32f10x.h"
#include "usart_dma_all.h"
#include "usart_ring_buff.h"
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"                               //任务头文件包含
#include "queue.h"                              // 使用消息队列要包含的头文件
#include "semphr.h"                             //信号量头文件包含
//**************************************************************************************



/*******************************************************************
********************************************************************
* 名称：void TIM6_Int_Init(u16 arr,u16 psc)
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
void TIM6_Int_Init(u16 arr,u16 psc)
{ 
	 NVIC_InitTypeDef NVIC_InitStructure;
	 TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	 
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);//TIM6时钟使能    
	 
	 //定时器TIM7初始化
	 TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值 
	 TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	 TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	 TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	 TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	 
	 
	 NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	 NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5 ;//抢占优先级0
	 NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //子优先级2
	 NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;   //IRQ通道使能
	 NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器
	 
	 TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
	 TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE ); //使能指定的TIM6中断,允许更新中断
	 TIM_Cmd(TIM6,DISABLE);//关闭定时器6
 
}
/*******************************************************************
********************************************************************
* 名称：void TIM6_IRQHandler(void)
* 功能：//定时器7中断服务程序
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
*  TIM6_Int_Init(100-1,7200-1);  //TIM6 10ms触发中断
*********************************************************************
*********************************************************************/   
void TIM6_IRQHandler(void)
{  
	char time_ring_buffer[RING_BUFFER_LEN] ={0};
	unsigned int receive_len = 0;
	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
	BaseType_t  pxHigherPriorityTaskWoken = pdFALSE ;  //任务切换返回值
	BaseType_t  usart1_queue_send_back ;               //保存入队函数返回值
	//===============================================
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)                        //是更新中断
	{   
		ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update  );                         //清除TIM6更新中断标志 		
		TIM_Cmd(TIM6, DISABLE);                                               //关闭TIM6    
		write_onebyte_to_ringbuffer(&usart1_receive_buffer,'\0');             // 写入字符串结束标识	 
        receive_len = get_ringbuffer_length(&usart1_receive_buffer);	      // 获取环形队列中的有效数据	
		read_ringbuffer(&usart1_receive_buffer,time_ring_buffer,receive_len); //将环形队列中的数据读取到一个数组中	                 
//		USART1_REVEIVE_OK = true;   	                                      // 收到数据的标志  false ： 没有收到数据   true ： 收到了数据
		if(usart1_receive_queue_handle != NULL )  // 队列存在
		{
			usart1_queue_send_back = xQueueSendFromISR(usart1_receive_queue_handle,time_ring_buffer,&pxHigherPriorityTaskWoken);
			if(usart1_queue_send_back == pdPASS)
			{
//				usart1_printf("%s\r\n","[** usart1 interrupt **] receive ok,queue send[== ok ==]");
			}
			else
			{
				usart1_printf("%s\r\n","[** time6 interrupt **] queue full,queue send[== fail ==]");
			}
		}
		// 判断是否在出中断的时候要进行任务切换
		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护
	}  
	
}





