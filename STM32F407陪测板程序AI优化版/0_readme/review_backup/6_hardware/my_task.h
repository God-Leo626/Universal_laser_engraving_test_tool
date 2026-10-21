#ifndef __START_TASK_H__
#define __START_TASK_H__
#include "stdbool.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"
#include "queue.h"                             // 使用消息队列要包含的头文件
#include "semphr.h"
enum state
{
	Value,
	ErrValue
};

extern bool    usart2_cmd_set_flag  ;    //  用于保存允许通过串口2 对nrf52832进行设置，初始化的时候默认可以设置（在设置开始扫描后禁止）
//================================start task===========================================
#define START_TASK_PRIO		1                      //任务优先级	
#define START_STK_SIZE 		400                    //任务堆栈大小
extern  TaskHandle_t  StartTask_Handler;           //任务句柄
void    start_task(void *pvParameters);            //任务函数

//=================================fixture start task==========================================
//#define FIXTURE_START_TASK_PRIO		7                       //任务优先级5
//#define FIXTURE_START_STK_SIZE 		1000                     //任务堆栈大小	1000
//extern  TaskHandle_t  Fixture_Start_Task_Handler;           //任务句柄
//void    fixture_start_task(void *pvParameters);             //任务函数

//=================================串口1 接收处理 task==========================================
#define USART1_RECEIVE_PRINT_TASK_PRIO		6                      //任务优先级
#define USART1_RECEIVE_PRINT_STK_SIZE 		1200*1                    //任务堆栈大小	700
extern  TaskHandle_t  Usart1_Receive_Print_Task_Handler;           //任务句柄
void    usart1_receive_print_task(void *pvParameters);             //任务函数

//=================================串口2 接收处理 task==========================================
#define USART2_RECEIVE_PRINT_TASK_PRIO		7                     //任务优先级
#define USART2_RECEIVE_PRINT_STK_SIZE 		1024*3                    //任务堆栈大小	700
extern  TaskHandle_t  Usart2_Receive_Print_Task_Handler;           //任务句柄
void    usart2_receive_print_task(void *pvParameters);             //任务函数
//=================================串口3 接收处理 task==========================================
//#define USART3_RECEIVE_PRINT_TASK_PRIO		2                        //任务优先级
//#define USART3_RECEIVE_PRINT_STK_SIZE 		700                      //任务堆栈大小	1500
//extern  TaskHandle_t  USART3_RECEIVE_Print_Task_Handler;             //任务句柄
//void    usart3_receive_print_task(void *pvParameters);               //任务函数

//=================================8266复连任务==========================================


//=================================工单配置任务==========================================


//=================================测试任务==========================================
//#define TEST_TASK_PRIO		            8                   //任务优先级7
//#define TEST_TASK_STK_SIZE 		        4200                //任务堆栈大小	1500
//extern  TaskHandle_t  TEST_Task_Handler;                    //任务句柄
//void    test_task(void *pvParameters);                      //任务函数



//=================================queue task==========================================

//===========================================================================
#define ONE_TIME_SIZE      (4*1024)                               // 用于一次从服务器端读取的固件字节数
extern QueueHandle_t       fixture_start_queue_handle ;           // 定义一个消息队列句柄变量，用于接收消息队列句柄
extern QueueHandle_t       adc1262_channel_change_queue_handle;   // 定义一个消息队列句柄变量，用于案件中发送消息，切换不同的测试通道
extern QueueHandle_t       usart1_receive_queue_handle ;          // 定义一个消息队列句柄变量，用于接收消息队列句柄，用于串口1通信
//extern QueueHandle_t       usart3_receive_queue_handle ;          // 定义一个消息队列句柄变量，用于接收消息队列句柄，用于串口3通信
//extern QueueHandle_t       test_queue_handle  ;                 // 定义一个消息队列句柄变量，用于通知测试任务运行

extern SemaphoreHandle_t  inform_esp8266_disconnect_semaphore_handle ;         // 创建信号量句柄变量，用于接收信号量句柄
extern SemaphoreHandle_t  inform_esp8266_connect_semaphore_handle  ;           // 创建信号量句柄变量，用于接收信号量句柄
extern SemaphoreHandle_t  inform_firmware_information_semaphore_handle  ;      // 创建信号量句柄变量，用于接收信号量句柄
extern SemaphoreHandle_t  inform_firmware_down_semaphore_handle ;              // 创建信号量句柄变量，用于接收信号量句柄

extern char threshold_l_global_string_buf[30] ;   // 用于存放设置阈值的double字符串 低值
extern char threshold_h_global_string_buf[30]  ;   // 用于存放设置阈值的double字符串 高值
//**************************************************************************************
void capture_string_between_two_string (char * source_string,char* destination_string,const char* begin_string,const char* end_string ) ;
bool capture_string_times_between_two_string (char * source_string,char* destination_string,const char* begin_string,const char* end_string ,u8 times) ;
void search_key(char * source_string,char* destination_string,char* key_name);
u32 my_atoi(char * source);

bool replace_string(char *result, char *source, char* delete_s1, char *add_s1,char *add_s2,char *add_s3,char *add_s4,char *add_s5);
void delete_char(char* str, char c);
void exchange_len_string(char* dest,char* source,u32 exchange_len);
bool capture_string_skip_n(char* dest,char* source,u32 skip_len);
void  intercept_data(char * source_string,char* destination_string,char* start_string,u32 number_of_data);

unsigned char  xor_fun(unsigned char *dat, unsigned char size);
void  capture_calibration_data_between_two_string (char * source_string,char * destination_string,const char* begin_string,const char* end_string ,u8 data_num) ;
#endif




