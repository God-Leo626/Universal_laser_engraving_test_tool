#include "ui_lcd.h"
#include "my_task.h"
#include "usart_dma_all.h"

//**************************************************************************************

//**************************************************************************************
int main(void)
{
	BaseType_t task_creak_status;  // 接收保存 任务创建函数的返回值
	ui_display_hardware_init();    // 显示测试ui及初始化各硬件
	//**************************************************************************************
	//=====================创建开始任务======================================================
    task_creak_status = xTaskCreate(
									(TaskFunction_t )start_task,            //任务函数
									(const char*    )"start_task",          //任务名称
									(uint16_t       )START_STK_SIZE,        //任务堆栈大小
									(void*          )NULL,                  //传递给任务函数的参数
									(UBaseType_t    )START_TASK_PRIO,       //任务优先级
									(TaskHandle_t*  )&StartTask_Handler);   //任务句柄  
	if(task_creak_status == pdPASS)
	{
		usart1_printf("start_[task] create  [>> ok <<]\r\n");
		vTaskStartScheduler();          //开启任务调度
		
	}
	else
	{
		usart1_printf("start_[task] create  [>> fail <<]\r\n");
		return -1;
	}

}


