#ifndef MY_TASK_H
#define MY_TASK_H

#include <stdbool.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* 栈深度单位为 StackType_t（STM32 上为 4 字节），不是字节数。 */
#define START_TASK_PRIO 1
#define START_STK_SIZE 400
#define USART1_RECEIVE_PRINT_TASK_PRIO 6
#define USART1_RECEIVE_PRINT_STK_SIZE 1200
#define USART2_RECEIVE_PRINT_TASK_PRIO 7
#define USART2_RECEIVE_PRINT_STK_SIZE (1024 * 3)

extern TaskHandle_t StartTask_Handler;
extern TaskHandle_t Usart1_Receive_Print_Task_Handler;
extern TaskHandle_t Usart2_Receive_Print_Task_Handler;

/* 开始扫描成功后为 false；按键依此禁止运行中修改配置。 */
extern volatile bool usart2_cmd_set_flag;

void start_task(void *pvParameters);
void usart1_receive_print_task(void *pvParameters);
void usart2_receive_print_task(void *pvParameters);

/* main.c 提供：关闭中断并停机，错误原因可通过调试器查看。 */
void board_fatal_error(const char *reason);

#endif
