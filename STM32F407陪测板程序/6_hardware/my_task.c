#include "my_task.h"
#include "usart_dma_all.h"

#include "io_bit.h"
#include "time3.h"

#include "power_ctrl.h"
#include "button.h"
#include "lcd.h"

#include "systick_delay_freertos.h"

#include "string.h"
#include "usart_ring_buff.h"

#include "buzzer.h"
#include "stm32f4xx_chip_id.h"
#include "w25q32.h"


#include "ctype.h"

#include "stdlib.h"

#include "limits.h" //

//#include "assert.h"

//===========================================================================
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"
#include "queue.h"                             // 使用消息队列要包含的头文件
#include "semphr.h"                            //信号量头文件包含

//#include "arithmetic.h"

#include "variable_global.h"
#include "dma.h"
#include "data_processing.h"
//===================任务句柄========================================================
TaskHandle_t  StartTask_Handler = NULL;                                  //开始任务句柄

TaskHandle_t  Fixture_Start_Task_Handler= NULL;                         //fixture start 任务句柄

TaskHandle_t  Usart1_Receive_Print_Task_Handler= NULL;                  //任务句柄
TaskHandle_t  Usart2_Receive_Print_Task_Handler= NULL;                  //任务句柄

int8_t  rssi_value = 0 ;                                                // 保存rssi的设置值(usrat1 中pc发送过来并保存在此)
uint8_t feature_value [8] = {0} ;                                       // 保存提取出来的特征值，最多8个字节
bool    usart1_to_usart2_rssi_setflag    = false ;                      // 保存rssi需要设置的标志  true ： 要设置  false 不需要设置
bool    rssi_set_ok_flag                 = false ;                      // 保存rssi 设置成功的标志   false= 设置失败  true = 设置ok

bool    usart1_to_usart2_feature_setflag6 = false ;                      // 保存freture需要设置的标志  true ： 要设置  false 不需要设置
bool    usart1_to_usart2_feature_setflag7 = false ;                      // 保存freture需要设置的标志  true ： 要设置  false 不需要设置
bool    usart1_to_usart2_feature_setflag8 = false ;                      // 保存freture需要设置的标志  true ： 要设置  false 不需要设置
bool    feature_set_ok_flag              = false ;                       // 保存freture 设置成功的标志   false= 设置失败  true = 设置ok

bool    only_len_set_flag              =  false ;                       //  保存需要设置字节序的标志  true  要设置， false 不需要设置
bool    start_scanning_set_flag          = false ;                        //  保存需要设置nrf52832开始扫描的标志  true  要设置， false 不需要设置
bool    use_default_set_flag             = false ;                        //  保存需要设置默认设置的标志  true  要设置， false 不需要设置
bool    mask_raw_data_set_flag           = false ;                        //  保存需要设置打开显示原始字符的标志  true  要设置， false 不需要设置
bool    usart2_cmd_set_flag              = true ;                         //  用于保存允许通过串口2 对nrf52832进行设置，初始化的时候默认可以设置（在设置开始扫描后禁止）

bool     mask_raw_data_mode_scanning_flag = false ;    //  用于保存在设置打开显示原始数据后，通过串口1输出原始数据，便于查看ble的特征值


unsigned char save_read_from_flash_feature_buf[31] = {0};                 //  保存从flash 中读取的特征值

unsigned int start_x      = 3 ;             //  保存进度条显示的x开始坐标
unsigned int end_x        = 0 ;             //  保存进度条显示的x结束坐标

bool pc_to_usart1_get_sn_ok_flag = false ;   // 保存pc获取mac +sn ok后，返回给下位机的一个标志  真= 数据获取ok， 假=获取失败

unsigned char only_global_pc_set_value = 0 ;  //  保存通过pc命令设置的唯一码值或唯一码的特征
unsigned char old_only_global_pc_set_value = 0 ;  //  保存通过pc命令设置的唯一码值或唯一码的特征 (匹配旧上位机解析规则)

bool  old_only_len_rule_flag = false ;        //  保存使用旧上位机还时新上位机，怎么决策解析数据的逻辑  true=  旧上位机  false = 新上位机
unsigned char pc_reback_cmd[7]   = {0x7F,0X07,0X00,0X80,0X00,0X00,0X00};  //  PC收到mac +sn数据并解析正确后返回给下位机的指令 （pc_reback_cmd【2】 = only_len）



//TaskHandle_t  USART3_RECEIVE_Print_Task_Handler= NULL;                  //串口2任务句柄

//TaskHandle_t  ESP_8266_RECONNECT_TASK_Task_Handler= NULL;               //8266 WIFI断开连接任务句柄

//TaskHandle_t  WORKNUMBER_CONFIG_TASK_Task_Handler= NULL;                //工单配置任务句柄




//=======================队列====================================================
QueueHandle_t      fixture_start_queue_handle = NULL;            // 定义一个消息队列句柄变量，用于接收消息队列句柄,用于治具下压通信

//QueueHandle_t      adc1262_channel_change_queue_handle = NULL;   // 定义一个消息队列句柄变量，用于案件中发送消息，切换不同的测试通道

//QueueHandle_t      usart3_receive_queue_handle = NULL;           // 定义一个消息队列句柄变量，用于接收消息队列句柄，用于串口3通信

//QueueHandle_t      worknumber_config_queue_handle = NULL ;       // 定义一个消息队列句柄变量，用于接收消息队列句柄，用于串口1通信

//QueueHandle_t      test_queue_handle = NULL ;                   // 定义一个消息队列句柄变量，用于通知测试任务运行
//=======================信号量====================================================


//SemaphoreHandle_t  inform_firmware_information_semaphore_handle = NULL ;      // 创建信号量句柄变量，用于接收信号量句柄
//SemaphoreHandle_t  inform_firmware_down_semaphore_handle = NULL ;             // 创建信号量句柄变量，用于接收信号量句柄


//**************************************************************************************


/*******************************************************************
********************************************************************
* 名称：void start_task(void *pvParameters)  开始任务
* 功能：创建其他任务
* 参数：无
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
void start_task(void *pvParameters)
{
	BaseType_t task_creak_status;            // 用于保存任务句柄
	
	//===========================================================================
    taskENTER_CRITICAL();                   //进入临界区
	

	//************************创建一个队列深度为1的队列(用于串口1)**************************************************************
//	worknumber_config_queue_handle = xQueueCreate(1,sizeof(u8));     // 用于在串口1接收到数据后入队消息
//	if(worknumber_config_queue_handle == NULL)
//	{
//		printf("worknumber_config_queue creat [ ^^ fail ^^ ]\r\n"); //创建失败时打印
//	}
//	else
//	{
//		printf("worknumber_config_queue creat [ ^^ ok ^^ ]\r\n"); //创建ok时打印
//	}
	
	
	//**********************创建队列(治具下压，上抬)****************************************************************
//	fixture_start_queue_handle =  xQueueCreate(1,sizeof(u8));
//	if(fixture_start_queue_handle == NULL) // 队列创建失败
//	{
//		usart1_printf("fixture_start_queue creat [ ^^ fail ^^ ]\r\n"); //创建失败时打印
//	}
//	else  // 队列创建ok
//	{
//		usart1_printf("fixture_start_queue creat [ ^^ ok ^^ ]\r\n"); //创建ok时打印
//	}
	
	//**********************创建队列(通道切换按键使用)****************************************************************
//	adc1262_channel_change_queue_handle =  xQueueCreate(1,sizeof(u8));
//	if(adc1262_channel_change_queue_handle == NULL) // 队列创建失败
//	{
//		usart1_printf("adc1262_channel_change_queue_handle creat [ ^^ fail ^^ ]\r\n"); //创建失败时打印
//	}
//	else  // 队列创建ok
//	{
//		usart1_printf("adc1262_channel_change_queue_handle creat [ ^^ ok ^^ ]\r\n"); //创建ok时打印
//	}


	//*************************创建fixture start task*************************************************************
//	task_creak_status = xTaskCreate(
//									(TaskFunction_t ) fixture_start_task,     	
//									(const char*    ) "fixture_start_task",   	
//									(uint16_t       ) FIXTURE_START_STK_SIZE, 
//									(void*          ) NULL,				
//									(UBaseType_t    ) FIXTURE_START_TASK_PRIO,	
//									(TaskHandle_t*  ) &Fixture_Start_Task_Handler); 
//	if(task_creak_status == pdPASS)
//	{
//		usart1_printf("fixture_start_[task] create [ >> ok << ]\r\n");
//	}
//	else
//	{
//		usart1_printf("fixture_start_[task] create [ >> fail << ]\r\n");
//	}
	
	//*************************创建test task*************************************************************
//	task_creak_status = xTaskCreate(
//									(TaskFunction_t ) test_task,     	
//									(const char*    ) "test_task",   	
//									(uint16_t       ) TEST_TASK_STK_SIZE, 
//									(void*          ) NULL,				
//									(UBaseType_t    ) TEST_TASK_PRIO,	
//									(TaskHandle_t*  ) &TEST_Task_Handler); 
//	if(task_creak_status == pdPASS)
//	{
//		usart1_printf("test_task create [ >> ok << ]\r\n");
//	}
//	else
//	{
//		usart1_printf("test_task create [ >> fail << ]\r\n");
//	}
//	
	//=====================创建串口1 接收处理任务 task======================================================
	task_creak_status = xTaskCreate(
									(TaskFunction_t ) usart1_receive_print_task,     	
									(const char*    ) "usart1_receive_print_task",   	
									(uint16_t       ) USART1_RECEIVE_PRINT_STK_SIZE, 
									(void*          ) NULL,				
									(UBaseType_t    ) USART1_RECEIVE_PRINT_TASK_PRIO,	
									(TaskHandle_t*  ) &Usart1_Receive_Print_Task_Handler); 
	if(task_creak_status == pdPASS)
	{
		usart1_printf("< usart1 >_receive_print_[task] create [>> ok <<]\r\n");
	}
	else
	{
		usart1_printf("< usart1 >_receive_print_[task] create [ >> fail <<]\r\n");
	}



	//===========================================================================
    task_creak_status = xTaskCreate(
									(TaskFunction_t ) usart2_receive_print_task,     	
									(const char*    ) "usart2_receive_print_task",   	
									(uint16_t       ) USART2_RECEIVE_PRINT_STK_SIZE, 
									(void*          ) NULL,				
									(UBaseType_t    ) USART2_RECEIVE_PRINT_TASK_PRIO,	
									(TaskHandle_t*  ) &Usart2_Receive_Print_Task_Handler); 
	if(task_creak_status == pdPASS)
	{
		usart1_printf("< usart2 >_receive_print_[task] create [>> ok <<]\r\n");
	}
	else
	{
		usart1_printf("< usart2 >_receive_print_[task] create [ >> fail <<]\r\n");
	}

	usart1_printf("************************************************\r\n");
	
	//===========================================================================
	
    vTaskDelete(StartTask_Handler); //删除开始任务
	//**************************************************************************************
	
	
    taskEXIT_CRITICAL();            //退出临界区
}








/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

void usart1_receive_print_task(void * pvParameters)
{
	u8 read_buf[USART1_REC_LEN] = {0};          //  用于usart1 从环形队列中读取数据后保存
    
    unsigned char xor_result    = 0 ;           //  保存异或校验值
    u8 prj_name_buf[50]         = {0};          //  保存项目名称
    
    u8 only_len_buf[3]          = {0};          //  保存解析出来的唯一码字符串
//    u8 only_hexstring_buf[6]       = {0};          //  保存解析出来的唯一码用于打印的十六进制字符串
    u32 string_len              = 0 ;           //  保存接收到的字符串的长度

//	BaseType_t   worknumber_config_squeue_back = errQUEUE_FULL;  // 保存入队函数的返回值     usart1_dma_tx_send

//    
    unsigned char  pc_rssi_cmd[]     = {0x7f, 0x07, 0x81, 0x00, 0x00 } ;   // pc发送设置rssi 的命令的前几个字节的固定格式
    unsigned char  pc_feature1_cmd[] = {0x7f, 0x0c, 0x82, 0x00, 0x00 } ;   // pc发送设置feature 的命令的前几个字节的固定格式
    unsigned char  pc_feature2_cmd[] = {0x7f, 0x0d, 0x82, 0x00, 0x00 } ;   // pc发送设置feature 的命令的前几个字节的固定格式
    unsigned char  pc_feature3_cmd[] = {0x7f, 0x0e, 0x82, 0x00, 0x00 } ;   // pc发送设置feature 的命令的前几个字节的固定格式
    
    
    
    
	while(1)
	{    
		if( USART1_REVEIVE_OK == true )          //  usart1 接收到了数据后，标志会为真
		{
			USART1_REVEIVE_OK = false;
            
            taskENTER_CRITICAL();               //  进入临界区
            
			string_len = get_ringbuffer_length_for_usart1(&usart1_receive_buffer) ;   // 获取串口1接收到的字符长度
            if(string_len >= 7)  // 所有上位机发送的指令当中，没有少于7个字节的，所以当缓冲区的长度大于等于7时，读取缓冲区
            {
                memset(read_buf,0,USART1_REC_LEN);  //清理空间
                read_ringbuffer_for_usart1(&usart1_receive_buffer,(char*)read_buf,string_len);  // 从串口1的环形缓冲器读取数据，并将数据保存在read_buf中
            }
            
            taskEXIT_CRITICAL();                //退出临界区
            
            
          //**************串口1 处理接收PC 发送过来的数据（rssi +特征值等）***************************************************************************************************
            if(string_len == 7)  //  命令总长度为7
            {            
                if ( xor_checksum_range(read_buf, 1, 5, &xor_result) == true )    //   计算收到的数据的异或校验值
                {
//                    usart1_printf("xor_result = %02x\r\n",xor_result);
                    if (xor_result == read_buf[6])                                // 计算出来的异或校验值和发送的异或校验值相等，命令发送正确
                    {
                        if ( hex_arr_compare(read_buf, pc_rssi_cmd, 5) == true )  //  命令的前5个字节符合命令要求
                        {  
                            rssi_value  =  read_buf[5] ;                          //  提取rssi值
                            usart1_to_usart2_rssi_setflag = true ;                //  rssi设置标志置位， 表示可以在usart2 任务中设置rssi值
                            for(char i = 0 ; i < 7; i++)
                            {
                                usart1_printf("%02x",read_buf[i]);
                                if( i == 6)
                                {
                                    usart1_printf("\r\n") ;
                                }                              
                            }
                            
//                            usart1_printf("rssi = %d\r\n",rssi_value);
                        }
                    }
                }    
            }
            //************** 串口1  处理接收pc 发送过来的设置特征值  【6个字节特征值指令】的指令***************************************************************************************************
            else if( string_len ==  12)  //  命令总长度=12
            {
                if ( hex_arr_compare(read_buf, pc_feature1_cmd, 5) == true )  //  命令的前5个字节符合命令要求 (特征值为6个字节)
                {
                     if ( xor_checksum_range(read_buf, 1, 10, &xor_result) == true )    //   计算收到的数据的异或校验值
                     {
                         if(xor_result == read_buf[11]) // 计算出来的异或值和命令中的值相当，表示命令发送正确
                         { 
                             memset(feature_value,'\0',sizeof(feature_value));  //  清零feature_value
                             for (char i = 0 ; i < 6; i++)  // 将命令中的 5,6,7,8,9,10 数组中的特征值保存
                             {
                                feature_value[i] = read_buf[i + 5] ;
                             }
                             usart1_to_usart2_feature_setflag6 = true; 
                             //  打印发送的内容
                             for(char i = 0 ; i < 12; i++)
                             {
                                usart1_printf("%02x",read_buf[i]);
                                if( i == 11)
                                {
                                    usart1_printf("\r\n") ;
                                }                              
                             }
                         }
                     }
                }
            }
             //************** 串口1  处理接收pc 发送过来的设置特征值  【7个字节特征值指令】的指令*****************
            else if( string_len ==  13)  //  命令总长度=13
            {
                if( hex_arr_compare(read_buf, pc_feature2_cmd, 5) == true) //  命令的前5个字节符合命令要求 (特征值为7个字节)
                {
                    if ( xor_checksum_range(read_buf, 1, 11, &xor_result) == true )    //   计算收到的数据的异或校验值
                     {
                         if(xor_result == read_buf[12]) // 计算出来的异或值和命令中的值相当，表示命令发送正确
                         { 
                             memset(feature_value,'\0',sizeof(feature_value));  //  清零feature_value
                             for (char i = 0 ; i < 7; i++)  // 将命令中的 5,6,7,8,9,10，11 数组中的特征值保存
                             {
                                feature_value[i] = read_buf[i + 5] ;
                             }
                             usart1_to_usart2_feature_setflag7 = true; 
                             //  打印发送的内容
                             for(char i = 0 ; i < 13; i++)
                             {
                                usart1_printf("%02x",read_buf[i]);
                                if( i == 12)
                                {
                                    usart1_printf("\r\n") ;
                                }                              
                             }
                         }
                     }                  
                }
            
            }
             //************** 串口1  处理接收pc 发送过来的设置特征值  【8个字节特征值指令】的指令*****************
            else if( string_len ==  14)  //  命令总长度=14
            {
                if( hex_arr_compare(read_buf, pc_feature3_cmd, 5) == true)  //  命令的前5个字节符合命令要求 (特征值为8个字节)
                {
                    if ( xor_checksum_range(read_buf, 1, 12, &xor_result) == true )    //   计算收到的数据的异或校验值
                     {
                         if(xor_result == read_buf[13]) // 计算出来的异或值和命令中的值相当，表示命令发送正确
                         { 
                             memset(feature_value,'\0',sizeof(feature_value));  //  清零feature_value
                             for (char i = 0 ; i < 8; i++)  // 将命令中的 5,6,7,8,9,10,11,12 数组中的特征值保存
                             {
                                feature_value[i] = read_buf[i + 5] ;
                             }
                             usart1_to_usart2_feature_setflag8 = true; 
                             //  打印发送的内容
                             for(char i = 0 ; i < 14; i++)
                             {
                                usart1_printf("%02x",read_buf[i]);
                                if( i == 13)
                                {
                                    usart1_printf("\r\n") ;
                                }                              
                             }
                         }
                     } 
                }
            }
            //******************* 处理pc 端发送过来的 设置 【唯一码值】   的处理指令**********************************************************************************************
            //*********************【当需要使用旧规则解析数据时， 要先发旧的cmd， 然后发新规则cmd 要怎么解析的命令】********************************************************************************************
            if(strstr((const char *)read_buf,"only_len_set=") != NULL )  // 设置唯一码是什么， 或者唯一码的长度（唯一码 < 0x83,表示唯一码的长度，否则表示特征）
            {
                taskENTER_CRITICAL();                                                                      //  进入临界区

                memset(only_len_buf,'\0',sizeof(only_len_buf));                                             // 清零
 
                str_intercept((const char *)read_buf, "only_len_set=","\r\n",(char *)only_len_buf, 3) ;  //  截取从   only_len_set= 到\r\n 之间的字符          
//                usart1_printf("only_len_buf = %s\r\n",only_len_buf); 
              
                
                // 将十六进制字符串，按照2个一组转换为十六进制数
                if (hex_str_to_bytes_by_len((const char *)only_len_buf, strlen((const char *)only_len_buf), (u8 *)&only_global_pc_set_value, 1) == 1)
                {
                    if(only_global_pc_set_value < 0x83) // 为什么是0x83，因为老上位机在镭雕接收这块是从0x83开始的，比0x83大的要按照老的规则解析 （小的使用新规则）
                    {
                        lcd_fill_color(85,261,117,277,Black);  // 清理显示区域
                        display_ascii_string(85,261,8,(u8*)"0X",Green,Black);  // 在 唯一码数据前面加0x， 表示时十六进制数据
                        display_ascii_string(101,261,8,(u8*)only_len_buf,Green,Black);                                 // 在lcd上显示 唯一码 十六进制字符串设置值
                        w25q32_flash_autosector_write_with_erase((u8 *)&only_global_pc_set_value,0x00000046,1,2) ;    // 将唯一码的保存在flash 中
                        w25q32_flash_autosector_write_with_erase((u8 *)"\r\n",0x00000047,2,2) ;                       // 在保存唯一码的后面继续写入回车换行，方便后面解析数据
                        usart1_printf("%s\r\n",read_buf);                                                             // 回显命令
//                        old_only_len_rule_flag = false ;   //  使用新的唯一码解析规则 新的cmd规则来解析数据
                    }
                    else //  匹配老规则
                    {
                        
                        old_only_global_pc_set_value = only_global_pc_set_value ;  // 保存旧上位机要使用的唯一码的 cmd
                        
                        lcd_fill_color(121,261,153,277,Black);  // 清理显示区域
                        display_ascii_string(121,261,8,(u8*)"0X",Green,Black);  // 在 唯一码数据前面加0x， 表示时十六进制数据
                        display_ascii_string(137,261,8,(u8*)only_len_buf,Green,Black);                                 // 在lcd上显示 唯一码 十六进制字符串设置值
                        w25q32_flash_autosector_write_with_erase((u8 *)&old_only_global_pc_set_value,0x00000049,1,2) ;    // 将唯一码的保存在flash 中
                        w25q32_flash_autosector_write_with_erase((u8 *)"\r\n",0x0000004A,2,2) ;                       // 在保存唯一码的后面继续写入回车换行，方便后面解析数据
                        usart1_printf("%s\r\n",read_buf);                                                             // 回显命令
                                
                        lcd_fill_color(5,414,317,426,Black);  // 清理掉以前老 cmd 设置失败时候的提示信息
                        old_only_len_rule_flag = true ;  //   使用旧上位机
                        
                    }
                                                                             
                }
                else
                {
                    lcd_fill_color(85,261,317,277,Black);  // 清理显示区域
                    display_ascii_string(85,261,8,(u8*)"ONLY_LEN SET ERROR",Red,Black);  
                    usart1_printf("[ONLY_LEN SET ERROR]\r\n");   
                }

                                                                          
                
                taskEXIT_CRITICAL();                
            }
             //******************* 处理pc 端发送过来的 设置  是否显示【 原始ble广播 】值的指令*************************************************************
            else if(strstr((const char *)read_buf,"mask_raw_data\r\n") != NULL )  // 打开原始数据显示
            {
                mask_raw_data_set_flag = true; 
                usart1_printf("%s\r\n",read_buf);
            }
            else if(strstr((const char *)read_buf,"use_default_settings\r\n") != NULL )   // 设置默认设置并开始扫描
            {
                use_default_set_flag = true ;
                usart1_printf("%s\r\n",read_buf);
            }
            else if(strstr((const char *)read_buf,"start_scanning\r\n") != NULL )  // 设置开始扫描
            {
                start_scanning_set_flag = true ;
                usart1_printf("%s\r\n",read_buf);
            }
            //********************关于 【项目名称】的设置， 直接在串口1里完成， 不需要nrf52832执行*********************************************************************************************
            else if(strstr((const char *)read_buf,"prj_set=") != NULL )  // 设置项目名称
            {
                memset(prj_name_buf,'\0',50) ;
                str_intercept((const char *)read_buf, "prj_set=","\r\n",(char *)prj_name_buf, 50) ;  // 截取项目名称
                
                if(strlen((const char*)prj_name_buf) <= 26 )  // 采用12的字号显示，320的位置，最多显示26个字符
                {
                    taskENTER_CRITICAL();               //  进入临界区
                    usart1_printf("PRO_NAME = %s\r\n",prj_name_buf);
                    lcd_fill_color(1,1,320,52,Black);  //刷新以前显示
                    display_ascii_string( (320 - strlen((const char*)prj_name_buf) * 12 ) / 2,14,12,(u8*) prj_name_buf,LimeGreen,Black);    // 居中显示prj name
                    
                    w25q32_flash_autosector_write_with_erase((u8 *)prj_name_buf,0x00000028,30,2) ;   // 将项目名的要求记录到flash中
                    w25q32_flash_autosector_write_with_erase((u8 *)"\r\n",0x00000028 + strlen((const char*)prj_name_buf),2,2) ;   // 将项目名后面继续写入\R\N (读取解析的时候需要\r\n)
                    taskEXIT_CRITICAL();                //退出临界区
                }
                else
                {
                    usart1_printf("NAME TOO LONG\r\n");
                    lcd_fill_color(1,1,320,52,Black);  //刷新以前显示
                    display_ascii_string(82 ,14,12,(u8*)"NAME TOO LONG",LimeGreen,Black);    // 居中显示prj name
                }
            }
            // =======================上位机收到mac +sn 信息后回复信息给下位机=====================
            else if(strstr((const char *)read_buf,"tesk_ok\r\n") != NULL )  // pc端获取了
            {
               pc_to_usart1_get_sn_ok_flag = true ;  //  pc 接收并处理mac +sn 无误了 （例如 ：传递给镭雕机了）                
            }
//            else
//            {
//                usart1_printf("cmd is error !\r\n");
//            }
            // =======================上位机收到mac +sn 信息后回复信息给下位机======================
            //  上位机解析到正确的mac+使能后，会返回 0x7f 07  cmd  80  00  00  fcs

            
           
            // 比较两个十六进制数组从开始到指定长度的内容是否相同（指定长度为 7，返回命令的总长度固定为7）
            // 接收到数据后，第一次对比看是否相同，由于第一次，对比的对象pc_reback_cmd={0x7F,0X07,0X00,0X80,0X00,0X00,0X00}
            // pc在正确回复的情况下，肯定是不会等于 {0x7F,0X07,0X00,0X80,0X00,0X00,0X00}，所以会执行else分支
            taskENTER_CRITICAL();               //  进入临界区
            if(hex_arr_compare(read_buf, pc_reback_cmd, 7) == true )
            {
                //  pc 接收并处理mac +sn 无误了 （例如 ：传递给镭雕机了） ,这个值是给接下来的下位机显示绿色界面与否的判断依据（true = 显示绿色）
                pc_to_usart1_get_sn_ok_flag = true ;  
            }
//            else // 在此分支里，更新要对比的对象pc_reback_cmd，第一次执行完成后，要对比的对象就固定下来了（从第二次开始，不会在进入else分支了）
//            {
//                if(cmd_select_flag == true)
//                {
//                    pc_reback_cmd[2] = old_only_global_pc_set_value ;  //  将通过pc设置的 唯一码老的  数据赋值给 pc_reback_cmd[2]
//                }
//                else
//                {
//                    pc_reback_cmd[2] = only_global_pc_set_value ;  //  将通过pc设置的 唯一码 新的  数据赋值给 pc_reback_cmd[2]
//                }
//                
//                u8 pc_reback_result = 0 ;  //  保存上位机返回的异或校验值
//                if (xor_checksum_range(pc_reback_cmd, 1, 6, &pc_reback_result) == true) // 下位机计算的异或校验值 成功
//                {
//                    pc_reback_cmd[6]  = pc_reback_result ;  //  将下位机自己计算的异或校验值赋值给命令   
//                }
//                // 第一次更新比较对象pc_reback_cmd 后，进行一次比较
//                if(hex_arr_compare(read_buf, pc_reback_cmd, 7) == true )
//                {
//                   pc_to_usart1_get_sn_ok_flag = true ;  //  pc 接收并处理mac +sn 无误了 （例如 ：传递给镭雕机了）  
//                }
//                for (char i = 0 ; i < 7; i++)
//                {
//                    usart1_printf("%02x",pc_reback_cmd[i]);
//                }
//            }
            taskEXIT_CRITICAL();                //退出临界区
            
            
          
                
			
			

		
		}

		vTaskDelay(pdMS_TO_TICKS(50));	//  
        
        //******************************** 【以下是通过lcd的显示，表示任务是否在正确的切换】 *********************************************************************************
        taskENTER_CRITICAL();               //  进入临界区
        end_x = end_x + 2  ;
        if( end_x  <= 318 )
        {
            lcd_fill_color(start_x , 430 , end_x , 436 , DarkBlue);  // 显示程序运行进度条
            start_x = end_x ;
        }
        else
        {  
           start_x = 3 ;
           end_x = 0 ;
           lcd_fill_color(3,430,318,436,Black);  // 清理掉上次的显示 （进度条）
        }
        taskEXIT_CRITICAL();                //退出临界区
		
	}

}


/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void usart2_receive_print_task(void * pvParameters)
{
    unsigned char cmd_buf[100] =  {0} ;          //  用于保存要发送的命令  字符串
    unsigned char mac_buf[32] = {0};             //  保存截取的mac唯一码  字符串
    unsigned char hex_number_mac_buf[16] = {0};  //  保存mac 命令的十六进制 （长度为字符串的一半即可）
    unsigned char sn_buf[15] = {0};              //  保存截取的sn 字符串
    unsigned char hex_number_sn_buf[8] = {0};    //  保存截取的sn 的十六进制  （长度为字符串的一半即可）
    unsigned char ch_buf [4] = {0};              //  保存截取的ch
    unsigned char rssi_buf[5] = {0};             //  保存截取的rssi
    unsigned char usart2_only_len_buf[4] = {0};        //  用于保存从flash 中读取的 唯一码特征值
    u8 read_buf[USART2_REC_LEN] = {0};          //  用于usart2 从环形队列中读取数据后保存
    u32 string_len = 0 ;                        //  保存接收到的字符长度
    unsigned char  set_rssi_ok_cmd_buf[]     = {0x7f,0x07,0x81,0x80,0x00,0x00,0x06}; //rssi 设置ok后返回值
    unsigned char  set_feature_ok_cmd_buf[]  = {0x7f,0x07,0x82,0x80,0x00,0x00,0x05}; // feature 设置ok后返回值
//    bool lcd_display_flag = false ;             //  保存显示状态，用于在现实mac ，sn时，切换不同的颜色显示
    
    bool rssi_set_ok_from_flash    = false ;   //  通过读取flash保存的内容设置rssi 成功标志  true = 设置ok
    bool feature_set_ok_from_flash = false ;   //  通过读取flash保存的内容设置feature 成功标志  true = 设置ok
   
    
    bool mac_sn_determine  = false ;   //  用于保存对接收到的数据是否时唯一的sn或者mac   真= 唯一
    bool one_time_display_flag =  false ;   // 用于保存显示控制（lcd显示一次结果后，在下一次接收到正确数据前不在刷新显示， false = 不刷新显示   true = 显示一次）
    
//    unsigned char  set_feature_ng_cmd_buf[]  = {0x7f,0x07,0x82,0x80,0x00,0x01,0x04}; // feature 设置NG后返回值
//    unsigned char  set_rssi_ng_cmd_buf[]     = {0x7f,0x07,0x81,0x80,0x00,0x01,0x07}; //rssi 设置NG后返回值
    unsigned char usart1_to_pc_snmac_cmd_buf[40] = {0x7f,0x00,0x00,0x00,0x00}; // 保存将要发送给 pc的 mac +sn 信息
    unsigned char usart1_to_pc_snmac_cmd_copy_buf[40] = {0};   // 保存发送给pc的命令备份
    while(1)
    {
        //**********************************NRF52832 接收广播并发送给PC*******************************************************************************
        if( USART2_REVEIVE_OK == true )         // usart2 接收到了数据后，标志会为真
		{
            USART2_REVEIVE_OK = false ;         // 将接收到数据的标志为置假
            
            
            taskENTER_CRITICAL();               //  进入临界区
            
            string_len = get_ringbuffer_length_for_usart1(&usart2_receive_buffer);
            
            memset(read_buf,'\0',USART2_REC_LEN);  //  清零read_buf，以备保存数据
            taskEXIT_CRITICAL();                //退出临界区
            if(usart2_cmd_set_flag == true)        //  这个标志表示没有开始ble接收，还在设置参数的阶段里，参数设置回复均较短，所以当有10字字符时就可以读取环形buf
            {
                if(string_len >= 10)
                {
//                    taskENTER_CRITICAL();               //  进入临界区
                    read_ringbuffer_for_usart1(&usart2_receive_buffer,(char*)read_buf,string_len); 
//                    taskENTER_CRITICAL();               //  进入临界区
                }
            }
            else  // 表示已经不是在参数设置环境里了，开始了ble广播接收，ble 广播接收的数据比较长
            {
                // [RSSI=-22] ---> [CH=37] ---> [tx_pwr=127] ---> [DECRYPT_MAC]=DC5475A87ACC ---> [DECRYPT_SN]=23025100000298
                if(string_len >= 212) // 为什么取212， 因为收到一条正确的广播数据长度是106，至少要收2条，就是 106*2 = 212
                {
//                    taskENTER_CRITICAL();               //  进入临界区
                    read_ringbuffer_for_usart1(&usart2_receive_buffer,(char*)read_buf,string_len); 
//                    taskENTER_CRITICAL();               //  进入临界区
                } 
            }
            
            
            
          
            
            if( string_len >= 10 && (mask_raw_data_mode_scanning_flag == false))  //  usart2 接收nrf52832的数据
            {
                //****************************处理接收到的ble广播包数据************************************************************************************
                
                taskENTER_CRITICAL();               //  进入临界区
                memset(mac_buf,'\0',32);
                //  str_extract_equal 函数按照截取规则汲取并判断mac是否均相等，并将mac保存在mac_buf）
                //  截取的数据都相等，表示此次接收的数据中，只接收到一个设备的广播
                // 判断接收到的数据中mac是否唯一
                mac_sn_determine = str_extract_equal((const char *)read_buf, "---> [DECRYPT_MAC]=", " ---> [",(char*)mac_buf, 32);
                
                taskEXIT_CRITICAL();                //退出临界区
                if( mac_sn_determine  ==  true) 
                {
                    //   ========= 【0】在列表位置显示 黄色的 mac 数据=========
                    lcd_fill_color(85,81,318,158,Black);  // 清除前面的显示数据
//                    display_ascii_string(85,81,8,(u8*) mac_buf,Yellow,Black);    // 显示  Yellow  的 mac
                    if(decoding_setting_flag == false)                               //  异或校验解析方式， 唯一码mac 只有 12个字节
                    {
                         display_ascii_string(85,81,8,(u8*) mac_buf,Yellow,Black);    //  在列表位置 显示  Yellow  的 mac
                    }
                    else // 明文解析方式 ，唯一码是 chip id  ，字符比较长， 有32个字符，显示小一些
                    {
                         display_ascii_string(85,83,6,(u8*) mac_buf,Yellow,Black);    //  在列表位置 显示  Yellow  的 mac
                    }
                    
                    //  抓取sn
                    
                    taskENTER_CRITICAL();               //  进入临界区
                    memset(sn_buf,'\0',15);
                    //  判断接收到的数据中的sn 是否唯一
                    mac_sn_determine = str_extract_equal((const char *)read_buf, "---> [DECRYPT_SN]=", "\r\n",(char*)sn_buf, 15);
                    taskEXIT_CRITICAL();                //退出临界区
                    if( mac_sn_determine  ==  true && (strlen((const char *)sn_buf) == 14)) // 截取的sn数据长度为 14
                    {
                        //
                        pc_to_usart1_get_sn_ok_flag = false ;    // 在开始前要清理掉可能上一次pc在超时后发送的正确回复并 将 pc_to_usart1_get_sn_ok_flag 置位
                        //   ========= 【1】关闭stm32的usart2， 在一定时间内，不在响应接受其他数据=========
                        usart2_disable  ();  // 关闭usart2，放置在处理数据的过程中收到其他产品的数据
                        //   ========= 【2】打开定时器，开始计时=========
                        time3_run(true);
                        
                        //   ========= 【3】 在列表位置显示黄色的 sn 数据====================
                                             
                        display_ascii_string(85,101,8,(u8*)sn_buf,Yellow,Black);   //  在列表位置显示 Yellow  的sn
                        //   ========= 【4】 在列表位置显示黄色的 ch 数据====================
                        memset(ch_buf,'\0',4);
                        
                        if(str_intercept((const char *)read_buf, "---> [CH=", "] ---> [tx_pwr",(char*)ch_buf, 4) == true) // 抓取ch，并显示 ble广播通道
                        {
                            display_ascii_string(85,121,8,(u8*)ch_buf,Yellow,Black); 
                            
                        }
                        //   ========= 【5】 在列表位置显示黄色的 RSSI 数据====================
                        memset(rssi_buf,'\0',5);
                        
                        if(str_intercept((const char *)read_buf, "[RSSI=", "] ---> [CH",(char*)rssi_buf, 5) == true)  // 抓取rssi并显示rssi
                        {
                            display_ascii_string(85,141,8,(u8*)rssi_buf,Yellow,Black); 
                        }
                        //   ========= 【6】 在凸显色块的位置 黄色的 sn 数据====================
                        
                        lcd_fill_color(2,283,318,349,Yellow);  // 显示 黄色块矩形  
                        display_ascii_string(46,300,16,(u8*)sn_buf,Black,Yellow);   //  突出显示 Yellow  的sn
                        
                        
                        //   ========= 【7】 发送mac + sn 给 pc====================
                       
//                        unsigned char usart1_to_pc_snmac_cmd_buf[40] = {0x7f,0x00,0x00,0x00,0x00}; // 保存将要发送给 pc的 mac +sn 信息
                        //  要发送的命令个数如下：
                        // 0x7f len  cmd  0x00  0x00         唯一码                        sn   rssi  fcs
                        //   1   1    1    1      1    only_global_pc_set_value            7     1     1  
                        usart1_to_pc_snmac_cmd_buf[1] =  5 + only_global_pc_set_value + 7 + 1 + 1  ;  // 要发送的命令总长度 len
                       
                        if(cmd_select_flag == true)  // 使用老的cmd 解析逻辑
                        {
                            usart1_to_pc_snmac_cmd_buf[2] =  old_only_global_pc_set_value ;  // 要发送的命令cmd(pc 通过串口设置的), 老的cmd 解析逻辑
                        }
                        else  // 使用新的cmd 解析逻辑
                        {
                            usart1_to_pc_snmac_cmd_buf[2] =  only_global_pc_set_value ;  // 要发送的命令cmd(pc 通过串口设置的), 新的cmd 解析逻辑
                        }
                        
                        
                        // 将mac 十六进制字符串---》 转换为 十六进制数
                        memset(hex_number_mac_buf,'\0',sizeof(hex_number_mac_buf));  //  清零
                        //   将mac 十六进制字符串 2个为一段 转换为十六进制数，保存在 hex_number_mac_buf  (only_global_pc_set_value*2 = mac中有效字符的个数)
                        if (hex_str_to_bytes_by_len((const char *)mac_buf, only_global_pc_set_value*2, hex_number_mac_buf, sizeof(hex_number_mac_buf))  == only_global_pc_set_value)
                        {
                            for(char i = 0 ; i < only_global_pc_set_value ; i++)
                            {
                                usart1_to_pc_snmac_cmd_buf[i+5] = hex_number_mac_buf[i];  // 唯一码
                            }
                        }
                       
                        memset(hex_number_sn_buf,'\0',sizeof(hex_number_sn_buf));
                        //  将sn 十六进制字符串 2个为一段 转换为十六进制数，保存在 hex_number_sn_buf （7*2 = 7个字节的sn，字符数量为2*7）
                        if (hex_str_to_bytes_by_len((const char *)sn_buf, 7*2, hex_number_sn_buf, sizeof(hex_number_sn_buf)) == 7)
                        {
                            for(char i = 0; i < 7 ; i++)
                            {
                                usart1_to_pc_snmac_cmd_buf[5 + only_global_pc_set_value + i] = hex_number_sn_buf[i] ; // sn
                            }
                        }
                        
                        //  要发送的rssi值
                        signed char hex_rssi_value = 0 ;  //  定义一个字节有符号变量，用于保存rssi
                        // 将带符号的十进制字符串， 转换为带符号的十六进制数
                        dec_str_to_s8((const char *)rssi_buf, &hex_rssi_value) ;
                        
                        //  将转换好的rssi的十六进制数赋值给命令字符串的正确位置
                        usart1_to_pc_snmac_cmd_buf[5 + only_global_pc_set_value +7 + 1 + 1 - 2] = hex_rssi_value;
                        //  计算将要发送的命令的异或校验值
                        // 0x7f len  cmd  0x00  0x00         唯一码                        sn   rssi  fcs
                        //   1   1    1    1      1    only_global_pc_set_value            7     1     1 
                        u8 xor_result = 0 ; // 保存要发送命令的异或校验值
                        xor_checksum_range( usart1_to_pc_snmac_cmd_buf, 1, 5 + only_global_pc_set_value +7 + 1 + 1 - 2, &xor_result) ;
                        
                        usart1_to_pc_snmac_cmd_buf[5 + only_global_pc_set_value +7 + 1 + 1 - 1] = xor_result ;
                        
                        //  将上面拼接好的十六进制数组发送给 pc
//                        usart1_printf("BUF[0]=%02X, BUF[1]=%02X\r\n", usart1_to_pc_snmac_cmd_buf[0], usart1_to_pc_snmac_cmd_buf[1]);
                        taskENTER_CRITICAL();               //  进入临界区
                        usart1_dma_tx_send(usart1_to_pc_snmac_cmd_buf , 5 + only_global_pc_set_value + 7 + 1 + 1);
                        memset(usart1_to_pc_snmac_cmd_copy_buf,'\0',40);
                        // 将要发送给pc的命令保存一份
                        memcpy(usart1_to_pc_snmac_cmd_copy_buf,usart1_to_pc_snmac_cmd_buf,usart1_to_pc_snmac_cmd_buf[1] );
                        //   ========= 【8】 打开结果显示开关==================== 
                        one_time_display_flag = true ;  //  显示一次结果的标识打开后，程序开始根据结果执行显示 （不超时，收到回复， 显示绿色， 超时显示红色）
                        lcd_fill_color(210,440,318,477,Black);  // 清屏上一次测试显示的时间，为接下来的新时间显示做准备
                        systick_delay_ms(500);  //  延时，让人看到黄色界面
                        taskEXIT_CRITICAL();                //退出临界区

                        
                         
                        
                        
                       
                    }
                    else // 检测到sn不唯一
                    {
                        display_ascii_string(8,389,6,(u8*)"Duplicated [SN] detected",Red,Black); 
                    }                 
                }
                //  当接收到的ble 广播包中，有不同的mac出现时
                else
                {
//                    display_ascii_string(8,375,6,(u8*)"Duplicated MAC detected",Red,Black); 
                    usart1_printf("%s\r\n",read_buf); // 输出read_buf，最多输出string_len   
                }
                 
                // ==============【处理按键发送“deconding_setting\r\n”】,nrf52832 返回信息 [ decoding_setting_ok ] ==============
                if(strstr((const char *)read_buf,"plaintext_set_ok") != NULL) // nrf52832 回复了 [ decoding_setting_ok ] 
                {
                    if( decoding_setting_flag == true ) // 明文 解析数据
                    {
                        lcd_fill_color(5,81,77,97,Black);  // 清理上次显示
                        display_ascii_string(5,81,8,(u8*) "CHIP ID :",Silver,Black);
                        lcd_fill_color(165,261,245,277,Black);  // 清理上次显示
                        display_ascii_string(165,261,8,(u8*)"PLAINTEXT",Green,Black);  //  明文解析数据
                    }
                }
                else if(strstr((const char *)read_buf,"cipnertext_set_ok") != NULL) // nrf52832 回复了 [ decoding_setting_ok ] 
                {
                    if( decoding_setting_flag == false ) // 密文 xor 解析数据
                    {
                        lcd_fill_color(5,81,77,97,Black);  // 清理上次显示
                        display_ascii_string(5,81,8,(u8*) "MAC     :",Silver,Black); 
                        lcd_fill_color(165,261,245,277,Black);  // 清理上次显示
                        display_ascii_string(165,261,8,(u8*)"CIPHERTEXT",Green,Black);  // 异或方式解析数据
                    }
                }
                //*********************** usart2 处理 上位机发送的指令设置******************************************************************************************
                // 【rssi 阈值设置成功】
                if(strstr((const char *)read_buf,"rssi_set_ok") != NULL) //  收到了rssi 设置ok的回复
                {
                    rssi_set_ok_flag = true ;   //  rssi  设置成功的标志置位 
                }
                else if(strstr((const char *)read_buf,"rssi is not a plain numeric value") != NULL)
                {
                    rssi_set_ok_flag = false ;   //  rssi  设置失败的标志置位  
                }
                // 【特征值设置ok】
                else if(strstr((const char *)read_buf,"feature_value = 0x") != NULL)
                {
                    feature_set_ok_flag = true ;   //  feature  设置ok的标志置位  feature_value = 0x 
                }
                
                //  【字节序设置成功】
//                else if(strstr((const char *)read_buf,"byte_order_set_ok") != NULL)  //字节序设置ok
//                {  
//                    display_ascii_string(85,261,8,(u8*)"SET OK",Green,Black);
//                    taskENTER_CRITICAL();               //  进入临界区
//                    w25q32_flash_autosector_write_with_erase((u8 *)"1",0x00000046,1,2) ;   // 将字节序的要求记录到flash中
//                    taskEXIT_CRITICAL();                //退出临界区
//                }
                // 【 ble 广播原始值显示设置 成功】
                else if(strstr((const char *)read_buf,"mask_raw_data_ok") != NULL)  // 原始数据显示输出设置ok
                {
                     display_ascii_string(85,201,8,(u8*)"MASK ON",Green,Black);   
                     mask_raw_data_mode_scanning_flag = true ; 
                }
                //  【使用默认设置 设置成功】
                else if(strstr((const char *)read_buf,"default_settings_ok") != NULL) // 按照默认设置开始扫描设置ok
                {
                     display_ascii_string(85,221,8,(u8*)"USE DEFAULT",Green,Black);   
                }
                //  【开始扫描设置成功】
                else if(strstr((const char *)read_buf,"start_scanning_ok") != NULL)  // 开始扫描设置ok
                {
                     display_ascii_string(85,241,8,(u8*)"START SCANNING",Green,Black);
                     lcd_fill_color(85,357,317,373,Black);  // cmd 设置模式并禁止，清除可以设置的显示
//                     feature_set_ok_from_flash = false ;
//                     rssi_set_ok_from_flash = false ; 
                     usart2_cmd_set_flag = false ;   //  在设置开始扫描后，不运行在进行参数的设置（要想设置，重启系统即可） 
                     mask_raw_data_mode_scanning_flag = false ;
                }
            }    
            //  处理 打开ble 原始数据后的值，通过串口1 输出 ble 接收到的数据
            else if(mask_raw_data_mode_scanning_flag == true)  //  当设置了 显示ble接收的原始数据后，mask_raw_data_mode_scanning_flag 为真
            {
                if(strstr((const char *)read_buf,"default_settings_ok") != NULL) // 按照默认设置开始扫描设置ok
                {
                     display_ascii_string(85,221,8,(u8*)"USE DEFAULT",Green,Black); 
                     display_ascii_string(85,241,8,(u8*)"START SCANNING",Green,Black);
                     lcd_fill_color(85,357,317,373,Black);  // cmd 设置模式并禁止，清除可以设置的显示
                }
                usart1_printf("%s\r\n", read_buf); 
              
  
            }
            
            
        }
        //*********************处理pc收到sn + mac 后，在规定时间内返回下位机 ********************************************************************************************
        if ( one_time_display_flag == true ) //  表示下位机发送过一次命令，接下来要根据pc的回复显示（绿色或者红色）
        {
            if (pc_reback_msg_to_usart1_timeout == false)  //  没有超时
            {
                //========= 在没有超时时间内 , 检测pc 时候有回复信息，并置位 pc_to_usart1_get_sn_ok_flag ====================
                if (pc_to_usart1_get_sn_ok_flag == true)                        // 表示在没有超时的时间内，收到了pc的有效回复 （串口1中置真）
                {
                    taskENTER_CRITICAL();               //  进入临界区
                    pc_to_usart1_get_sn_ok_flag = false ;                            // 重新将pc 返回有效小的标志为置假
                    if(decoding_setting_flag == false)                               //  异或校验解析方式， 唯一码mac 只有 12个字节
                    {
                         display_ascii_string(85,81,8,(u8*) mac_buf,Green,Black);    //  在列表位置 显示  Green  的 mac
                    }
                    else // 明文解析方式 ，唯一码是 chip id  ，字符比较长， 有32个字符，显示小一些
                    {
                         display_ascii_string(85,83,6,(u8*) mac_buf,Green,Black);    //  在列表位置 显示  Green  的 mac
                    }
                   
                    display_ascii_string(85,101,8,(u8*)sn_buf,Green,Black);     //  在列表位置 显示  Green  的 sn
                    display_ascii_string(85,121,8,(u8*)ch_buf,Green,Black);     //  在列表位置 显示  Green  的 ch
                    display_ascii_string(85,141,8,(u8*)rssi_buf,Green,Black);  //  在列表位置 显示  Green  的 rssi
                    
                    lcd_fill_color(2,283,318,349,Green);                        //  显示 绿色块矩形
                    display_ascii_string(46,300,16,(u8*)sn_buf,Black,Green);    //  凸显 Green  的sn
                    
                    usart2_enable();                                            //  打开usart2
                    time3_run(false);                                           //  关闭定时器
                    pc_reback_msg_to_usart1_timeout = false ;                   // 将定时器超时的标志置假 （没有超时）
                    time3_count1 = 0 ;                                          //  将定时器里用于累加计数的变量清零，方便下次基础从0 开始计数
                    one_time_display_flag = false ;                             //  执行一次显示后，不在显示，将标志置假 （不在显示）
                    taskEXIT_CRITICAL();                                        //退出临界区
                }
            }
            else  // 超时了，没有收到pc的回复
            {
                taskENTER_CRITICAL();                                     //  进入临界区
                pc_reback_msg_to_usart1_timeout = false ;                 // 从新开始， 定时器恢复为没有超时
                pc_to_usart1_get_sn_ok_flag = false ;                     // pc是否有返回正确消息的标志  真= 有返回， 假= 没有返回
                
//                display_ascii_string(85,81,8,(u8*) mac_buf,Red,Black);    //  在列表位置 显示  Red  的 mac
                if(decoding_setting_flag == false)                               //  异或校验解析方式， 唯一码mac 只有 12个字节
                {
                     display_ascii_string(85,81,8,(u8*) mac_buf,Red,Black);    //  在列表位置 显示  Green  的 mac
                }
                else // 明文解析方式 ，唯一码是 chip id  ，字符比较长， 有32个字符，显示小一些
                {
                     display_ascii_string(85,83,6,(u8*) mac_buf,Red,Black);    //  在列表位置 显示  Green  的 mac
                }
                display_ascii_string(85,101,8,(u8*)sn_buf,Red,Black);     //  在列表位置 显示  Red  的 sn
                display_ascii_string(85,121,8,(u8*)ch_buf,Red,Black);     //  在列表位置 显示  Red  的 ch
                display_ascii_string(85,141,8,(u8*)rssi_buf,Red,Black);   //  在列表位置 显示  Red  的 rssi
                
                lcd_fill_color(2,283,318,349,Red);                        //  显示 绿色块矩形
                display_ascii_string(46,300,16,(u8*)sn_buf,Black,Red);    //  凸显 Green  的sn
                
                usart2_enable();                                          //  打开usart2
                time3_run(false);                                         //  关闭定时器
                time3_count1 = 0 ;                                        //  将定时器里用于累加计数的变量清零，方便下次基础从0 开始计数
                one_time_display_flag = false ;                           //  执行一次显示后，不在显示，将标志置假 （不在显示）
                taskEXIT_CRITICAL();                                      //退出临界区
            }
//            systick_delay_ms(500); 
        
        }
       
        // 第一次发送命令给pc 2s            没有超时                                       pc没有回复                                 允许一次显示
        if((time3_count1 % 4) == 0 && (pc_reback_msg_to_usart1_timeout == false) && (pc_to_usart1_get_sn_ok_flag == false) && (one_time_display_flag == true))
        {
            taskENTER_CRITICAL();               //  进入临界区
            usart1_dma_tx_send(usart1_to_pc_snmac_cmd_copy_buf , 5 + only_global_pc_set_value + 7 + 1 + 1);
            taskEXIT_CRITICAL();                                        //退出临界区
        }
        
        
        //*********************命令参数设置********************************************************************************************
        //*********************命令参数设置********************************************************************************************
        //*********************命令参数设置********************************************************************************************
        if (usart2_cmd_set_flag == true)  // 上电初始化后，usart2_cmd_set_flag标志默认为真，表示允许设置参数  （发送开始扫描后失效）
        {
            //===================设置RSSI ============================
            if(usart1_to_usart2_rssi_setflag == true)                    // 当标志为真时，表示需要设置rssi
            {
                memset(cmd_buf,'\0',sizeof(cmd_buf));                    // 将cmd_buf清零
                sprintf((char *)cmd_buf,"rssi_set=%d\r\n",rssi_value) ;  // 将要设置的rssi值 转换为字符，由usart2发送给 nrf52832
               
                usart2_printf("%s",cmd_buf);                             // 串口2发送设置rssi的指令给nrf52832 例如： rssi_set=-40
//                usart1_printf("rssi-->%s",cmd_buf);
                for( char i = 0 ; i < 3 ; i++ )
                {
                    vTaskDelay(pdMS_TO_TICKS(150));	//  延时等待 usart2任务接收到信息，处理信息
                    
                    if( USART2_REVEIVE_OK == true )  // 收到信息
                    {
                        USART2_REVEIVE_OK = false;
                        taskENTER_CRITICAL();
                        u32 check_len = get_ringbuffer_length_for_usart1(&usart2_receive_buffer);
                        memset(read_buf, '\0', USART2_REC_LEN);
                        read_ringbuffer_for_usart1(&usart2_receive_buffer, (char*)read_buf, check_len);
                        taskEXIT_CRITICAL();
                        
                        if(check_len >= 10)
                        {
                            if(strstr((const char *)read_buf, "rssi_set_ok") != NULL) // 收到了期待的值，表示rssi设置ok
                            {
                                rssi_set_ok_flag = true;  // 将rssi 设置ok的标志置位
                            }
                                
                            else if(strstr((const char *)read_buf, "feature_value = 0x") != NULL)  // 如果收到的是 特征值设置的期待值
                            {
                                feature_set_ok_flag = true;   //  将特征值设置ok的标志置位
                            }     
                        }
                    }
                    
                    if( rssi_set_ok_flag == true )  //  rssi 设置ok的标志置位， rssi设置ok
                    {
                        usart1_to_usart2_rssi_setflag = false ;              // rssi 设置成功后，需要将设置rssi的标志置假
                        rssi_set_ok_flag = false ;                           // rssi 设置成功后，将标志置假
                        usart1_dma_tx_send(set_rssi_ok_cmd_buf,7) ;          // 串口1 发送rssi成功返回值给pc
                       
                        memset(cmd_buf,'\0',sizeof(cmd_buf));   
                        sprintf((char *)cmd_buf,"%d",rssi_value) ;
                        lcd_fill_color(85,161,317,177,Black);  // 清除上一次的显示内容
                        display_ascii_string(85,161,8,(u8*)cmd_buf,Green,Black);   //  在lcd上显示设置的rssi值
                        
                        usart1_printf("\r\n%s\r\n",cmd_buf);
                        
                        
                        taskENTER_CRITICAL();               //  进入临界区
                        // 将设置的rssi值保存在flash中，下次上电直接读取后运行
                        w25q32_flash_autosector_write_with_erase((u8 *) &rssi_value,0x00000000,1,2) ;   // 将RSSI的要求记录到flash中
                        taskEXIT_CRITICAL();                //退出临界区    
                        break;
                    }
                }
                if(usart1_to_usart2_rssi_setflag == true)
                {
                    usart1_to_usart2_rssi_setflag = false ;
                    usart1_printf("rssi set timeout\r\n");
                }
                
            
            }
            //*************  处理  [ 特征值 ]****************************************************************************************************
            else if(usart1_to_usart2_feature_setflag6 == true)                 // 当标志为真时，表示需要设置feature 为6个字符
            {
                char out_buf[12] = {0};
                hex_arr_to_str(feature_value, 6, out_buf);                     // 将特征值按照十六进制字符保存在out_buf中
                memset(cmd_buf,'\0',sizeof(cmd_buf));                          // 将cmd_buf清零
                sprintf((char *)cmd_buf,"feature_value_set=%s\r\n",out_buf) ;  // 将要设置的feature值 转换为字符，由usart2发送给 nrf52832
                usart2_printf("%s",cmd_buf);                                   // 发送设置特征值的命令
//                usart1_printf("feature-->%s",cmd_buf);
                for ( char i = 0 ; i < 3; i++ )
                {
                    vTaskDelay(pdMS_TO_TICKS(150));	  // 上面发送命令后等待延时，让串口接收中断处理数据
                    if( USART2_REVEIVE_OK == true )   // 收到数据
                    {
                        USART2_REVEIVE_OK = false;    // 收到数据标志置假
                        taskENTER_CRITICAL();
                        u32 check_len = get_ringbuffer_length_for_usart1(&usart2_receive_buffer);
                        memset(read_buf, '\0', USART2_REC_LEN);
                        read_ringbuffer_for_usart1(&usart2_receive_buffer, (char*)read_buf, check_len);
                        taskEXIT_CRITICAL();
                        
                        if(check_len >= 10)
                        {
                            if(strstr((const char *)read_buf, "rssi_set_ok") != NULL)
                            {
                                rssi_set_ok_flag = true;
                            }
                                
                            else if(strstr((const char *)read_buf, "feature_value = 0x") != NULL)
                            {
                                feature_set_ok_flag = true;
                            }    
                        }
                    }
                    if( feature_set_ok_flag == true )
                    {
                        usart1_to_usart2_feature_setflag6 = false ;              // rssi 设置成功后，需要将设置rssi的标志置假
                        feature_set_ok_flag = false ;                            // rssi 设置成功后，将标志置假
                        usart1_dma_tx_send(set_feature_ok_cmd_buf,7) ;           // 串口1 发送feature成功返回值给pc
                        lcd_fill_color(85,181,317,197,Black);  // 清除上一次的显示内容
                        display_ascii_string(85,181,8,(u8*)out_buf,Green,Black); // 在 lcd上显示 特征值
                        usart1_printf("\r\n%s\r\n",out_buf);                     // 串口输出特征值
                        
                        taskENTER_CRITICAL();               //  进入临界区
                        w25q32_flash_autosector_write_with_erase((u8 *)out_buf,0x00000009,strlen(out_buf),2) ;   // 将 特征值 记录到flash中
                        w25q32_flash_autosector_write_with_erase((u8 *) "\r\n",0x00000009 + strlen(out_buf),2,2) ;   // 在 特征值后后面加入\r\n 记录到flash中
                        taskEXIT_CRITICAL();                //退出临界区  
                        break ;
                    }
                }
                if(usart1_to_usart2_feature_setflag6 == true)
                {
                    usart1_to_usart2_feature_setflag6 = false ;
                    usart1_printf("feature set timeout\r\n");
                }
                
                
            }
            else if(usart1_to_usart2_feature_setflag7 == true)            // 当标志为真时，表示需要设置feature 为7个字符
            {
                char out_buf[14] = {0};
                hex_arr_to_str(feature_value, 7, out_buf);                     // 将特征值按照十六进制字符保存在out_buf中
                memset(cmd_buf,'\0',sizeof(cmd_buf));                          // 将cmd_buf清零
                sprintf((char *)cmd_buf,"feature_value_set=%s\r\n",out_buf) ;  // 将要设置的feature值 转换为字符，由usart2发送给 nrf52832
                usart2_printf("%s",cmd_buf);   // 发送设置特征值的命令
//                usart1_printf("feature-->%s",cmd_buf);
                for(char i = 0 ; i < 3; i++ )
                {
                    vTaskDelay(pdMS_TO_TICKS(150));	
                    if( USART2_REVEIVE_OK == true )
                    {
                        USART2_REVEIVE_OK = false;
                        taskENTER_CRITICAL();
                        u32 check_len = get_ringbuffer_length_for_usart1(&usart2_receive_buffer);
                        memset(read_buf, '\0', USART2_REC_LEN);
                        read_ringbuffer_for_usart1(&usart2_receive_buffer, (char*)read_buf, check_len);
                        taskEXIT_CRITICAL();
                        
                        if(check_len >= 10)
                        {
                            if(strstr((const char *)read_buf, "rssi_set_ok") != NULL)
                            {
                                rssi_set_ok_flag = true;
                            }
                                
                            else if(strstr((const char *)read_buf, "feature_value = 0x") != NULL)
                            {
                                feature_set_ok_flag = true;
                            }
                                
                        }
                    }
                    if( feature_set_ok_flag == true )
                    {
                        usart1_to_usart2_feature_setflag7 = false ;              // rssi 设置成功后，需要将设置rssi的标志置假
                        feature_set_ok_flag = false ;                            // rssi 设置成功后，将标志置假
                        usart1_dma_tx_send(set_feature_ok_cmd_buf,7) ;           // 串口1 发送feature成功返回值给pc
                        
                        lcd_fill_color(85,181,317,197,Black);  // 清除上一次的显示内容
                        display_ascii_string(85,181,8,(u8*)out_buf,Green,Black);
                        usart1_printf("\r\n%s\r\n",out_buf);
                        
                        taskENTER_CRITICAL();               //  进入临界区
                        w25q32_flash_autosector_write_with_erase((u8 *)out_buf,0x00000009,strlen(out_buf),2) ;   // 将 特征值 记录到flash中
                        w25q32_flash_autosector_write_with_erase((u8 *) "\r\n",0x00000009 + strlen(out_buf),2,2) ;   // 在 特征值后后面加入\r\n 记录到flash中
                        taskEXIT_CRITICAL();                //退出临界区  
                        break ; 
                    }  
                }
                if(usart1_to_usart2_feature_setflag7 == true)
                {
                    usart1_to_usart2_feature_setflag7 = false ;
                    usart1_printf("feature set timeout\r\n");
                }
                
                
            }
            else if(usart1_to_usart2_feature_setflag8 == true)            // 当标志为真时，表示需要设置feature  为8个字符
            {
                char out_buf[16] = {0};
                hex_arr_to_str(feature_value, 8, out_buf);                     // 将特征值按照十六进制字符保存在out_buf中
                memset(cmd_buf,'\0',sizeof(cmd_buf));                          // 将cmd_buf清零
                sprintf((char *)cmd_buf,"feature_value_set=%s\r\n",out_buf) ;  // 将要设置的feature值 转换为字符，由usart2发送给 nrf52832
                usart2_printf("%s",cmd_buf);   // 发送设置特征值的命令
//                usart1_printf("feature-->%s",cmd_buf);
                for (char i = 0 ; i < 3 ; i++ )
                {
                    vTaskDelay(pdMS_TO_TICKS(150));	
                    if( USART2_REVEIVE_OK == true )
                    {
                        USART2_REVEIVE_OK = false;
                        taskENTER_CRITICAL();
                        u32 check_len = get_ringbuffer_length_for_usart1(&usart2_receive_buffer);
                        memset(read_buf, '\0', USART2_REC_LEN);
                        read_ringbuffer_for_usart1(&usart2_receive_buffer, (char*)read_buf, check_len);
                        taskEXIT_CRITICAL();
                        
                        if(check_len >= 10)
                        {
                            if(strstr((const char *)read_buf, "rssi_set_ok") != NULL)
                            {
                                rssi_set_ok_flag = true;
                            }
                                
                            else if(strstr((const char *)read_buf, "feature_value = 0x") != NULL)
                            {
                                feature_set_ok_flag = true;
                            }
                                
                        }
                    }
                    if( feature_set_ok_flag == true )
                    {
                        usart1_to_usart2_feature_setflag8 = false ;              // rssi 设置成功后，需要将设置rssi的标志置假
                        feature_set_ok_flag = false ;                            // rssi 设置成功后，将标志置假
                        usart1_dma_tx_send(set_feature_ok_cmd_buf,7) ;           // 串口1 发送feature成功返回值给pc
                        
                        lcd_fill_color(85,181,317,197,Black);  // 清除上一次的显示内容
                        display_ascii_string(85,181,8,(u8*)out_buf,Green,Black);
                        usart1_printf("\r\n%s\r\n",out_buf);
                        
                        taskENTER_CRITICAL();               //  进入临界区
                        w25q32_flash_autosector_write_with_erase((u8 *)out_buf,0x00000009,strlen(out_buf),2) ;   // 将 特征值 记录到flash中
                        w25q32_flash_autosector_write_with_erase((u8 *) "\r\n",0x00000009 + strlen(out_buf),2,2) ;   // 在 特征值后后面加入\r\n 记录到flash中
                        taskEXIT_CRITICAL();                //退出临界区  
                        break ; 
                    }
                }
                if(usart1_to_usart2_feature_setflag8 == true)
                {
                    usart1_to_usart2_feature_setflag8 = false ;
                    usart1_printf("feature set timeout\r\n");
                }
                 
            }
//            else if(byte_order_set_flag == true)                  //  当标志为真的时候，表示要设置mac显示的字节序
//            {
//                byte_order_set_flag = false ;
//                usart2_printf("%s","byte_order_set\r\n");         //  发送 mac 的显示字节序  的命令
//                
//            }
            else if(mask_raw_data_set_flag == true)               //  当标志为真的时候，表示要设置 ble 原始值显示
            {
                mask_raw_data_set_flag = false ;
                usart2_printf("%s","mask_raw_data\r\n");          //  发送 ble 原始值显示 命令
            }
            else if(use_default_set_flag == true)                 //  当标志为真的时候，表示要设置mac显示的字节序
            {
                use_default_set_flag = false ;
                usart2_printf("%s","use_default_settings\r\n");   //  发送设置 默认 扫描 的命令
            }
            else if( start_scanning_set_flag == true )            //  当标志为真的时候，表示要设置开始扫描指令
            {
                start_scanning_set_flag = false ;
                usart2_printf("%s","start_scanning\r\n");        //  发送开始扫描命令
            }
            
            
            
            
            //******************** 使用flash 里保存的参数设置NRF52832*********************************************************************************************
            //******************** 使用flash 里保存的参数设置NRF52832*********************************************************************************************
            //******************** 使用flash 里保存的参数设置NRF52832*********************************************************************************************
            else if( (cmd_set_flag_from_buttom == true )    // cmd_set_flag_from_buttom 在按键中断中置为 true （表示要使用flash中断中的默认值，不现场设置）
                             && (usart1_to_usart2_rssi_setflag       == false) 
                             && (usart1_to_usart2_feature_setflag6   == false) 
                             && (usart1_to_usart2_feature_setflag7   == false)
                             && (usart1_to_usart2_feature_setflag8   == false))  //  上述为flash 的为假，表示没有现场设置，或设置动作结束
            {
                //******************从flash中读取  【 1 rssi 】  ***************************************************
                memset(SPI1_FLASH_BUF,'\0',4096);
                taskENTER_CRITICAL();
                w25q32_flash_read(SPI1_FLASH_BUF,0x00000000,1,2);  // rssi阈值读取 （0----  -105）
                taskEXIT_CRITICAL();
                if( ((int8_t)SPI1_FLASH_BUF[0] <= 0) && ((int8_t)SPI1_FLASH_BUF[0] >= -105)) // 将读取到的数据转换为由符号数并判断
                {
//                    save_read_from_flash_rssi =  (int8_t)SPI1_FLASH_BUF[0];
                    memset(cmd_buf,'\0',sizeof(cmd_buf));                    // 将cmd_buf清零
                    sprintf((char *)cmd_buf,"rssi_set=%d\r\n",(int8_t)SPI1_FLASH_BUF[0]) ;  // 将要设置的rssi值 转换为字符，由usart2发送给 nrf52832
                    taskENTER_CRITICAL();
                    usart2_printf("%s",cmd_buf);                             // 串口2发送设置rssi的指令给nrf52832 例如： rssi_set=-40
                     taskEXIT_CRITICAL();
                    for( char i = 0 ; i < 3 ; i++ )
                    {
                        vTaskDelay(pdMS_TO_TICKS(150));	//  延时等待 usart2任务接收到信息，处理信息
                        
                        if( USART2_REVEIVE_OK == true )  // 收到信息
                        {
                            USART2_REVEIVE_OK = false;
                            
                            taskENTER_CRITICAL();
                            u32 check_len = get_ringbuffer_length_for_usart1(&usart2_receive_buffer);
                            memset(read_buf, '\0', USART2_REC_LEN);
                            read_ringbuffer_for_usart1(&usart2_receive_buffer, (char*)read_buf, check_len);
                            taskEXIT_CRITICAL();
                            
                            if(check_len >= 10)
                            {
                                if(strstr((const char *)read_buf, "rssi_set_ok") != NULL) // 收到了期待的值，表示rssi设置ok
                                {
                                    rssi_set_ok_flag = true;  // 将rssi 设置ok的标志置位
                                }
                                    
                                else if(strstr((const char *)read_buf, "feature_value = 0x") != NULL)  // 如果收到的是 特征值设置的期待值
                                {
                                    feature_set_ok_flag = true;   //  将特征值设置ok的标志置位
                                }     
                            }
                        }
                        
                        if( rssi_set_ok_flag == true )                           //  rssi 设置ok的标志置位， rssi设置ok
                        {
                            usart1_to_usart2_rssi_setflag = false ;              // rssi 设置成功后，需要将设置rssi的标志置假
                            rssi_set_ok_flag = false ;                           // rssi 设置成功后，将标志置假
                            rssi_set_ok_from_flash = true ;                     // 通过flash 内读取rssi值设置ok
                            usart1_dma_tx_send(set_rssi_ok_cmd_buf,7) ;          // 串口1 发送rssi成功返回值给pc
                           
                            memset(cmd_buf,'\0',sizeof(cmd_buf));   
                            sprintf((char *)cmd_buf,"%d",(int8_t)SPI1_FLASH_BUF[0]) ;
                            lcd_fill_color(85,161,317,177,Black);  // 清除上一次的显示内容
                            display_ascii_string(85,161,8,(u8*)cmd_buf,Green,Black);   //  在lcd上显示设置的rssi值
                            
                            usart1_printf("\r\n%s\r\n",cmd_buf);
                            
                            
                       
                            break;
                        }
                    }
                    
                   
                }
                
                
                //******************从flash中读取【 2 feature】   *****************************************************
                taskENTER_CRITICAL();
                memset(SPI1_FLASH_BUF,'\0',4096);
                w25q32_flash_read(SPI1_FLASH_BUF,0x00000009,31,2); // feature 特征值读取
                taskEXIT_CRITICAL();
                
                if (str_extract_before_first((const char *)SPI1_FLASH_BUF, "\r\n", (char*)save_read_from_flash_feature_buf, 31)  == true)  // 找到了有效字符
                {
                    memset(cmd_buf,'\0',sizeof(cmd_buf));                          // 将cmd_buf清零
                    sprintf((char *)cmd_buf,"feature_value_set=%s\r\n",save_read_from_flash_feature_buf) ;  // 将要设置的feature值 转换为字符，由usart2发送给 nrf52832
                    usart2_printf("%s",cmd_buf);                                   // 发送设置特征值的命令  ,例如 feature_value_set=0201051bffca0a
                    
                    for(char i = 0 ; i < 3; i++ )
                    {
                        vTaskDelay(pdMS_TO_TICKS(150));	
                        if( USART2_REVEIVE_OK == true )
                        {
                            USART2_REVEIVE_OK = false;
                            
                            taskENTER_CRITICAL();
                            u32 check_len = get_ringbuffer_length_for_usart1(&usart2_receive_buffer);
                            memset(read_buf, '\0', USART2_REC_LEN);
                            read_ringbuffer_for_usart1(&usart2_receive_buffer, (char*)read_buf, check_len);
                            taskEXIT_CRITICAL();
                            
                            if(check_len >= 10)
                            {
                                if(strstr((const char *)read_buf, "rssi_set_ok") != NULL)
                                {
                                    rssi_set_ok_flag = true;
                                }
                                    
                                else if(strstr((const char *)read_buf, "feature_value = 0x") != NULL)
                                {
                                    feature_set_ok_flag = true;
                                }
                                    
                            }
                        }
                        if( feature_set_ok_flag == true )
                        {
                            usart1_to_usart2_feature_setflag6 = false ;              // 特征值 设置成功后，需要将设置标志置假
                            usart1_to_usart2_feature_setflag7 = false ;
                            usart1_to_usart2_feature_setflag8 = false ;
                            feature_set_ok_flag = false ;                            // 特征值设置成功后，将标志置假
                            feature_set_ok_from_flash = true ;                     // 通过flash 内读取feature值设置ok
                            usart1_dma_tx_send(set_feature_ok_cmd_buf,7) ;           // 串口1 发送feature成功返回值给pc
                            
                            lcd_fill_color(85,181,317,197,Black);  // 清除上一次的显示内容
                            display_ascii_string(85,181,8,(u8*)save_read_from_flash_feature_buf,Green,Black);
                            usart1_printf("\r\n%s\r\n",save_read_from_flash_feature_buf);

                            break ; 
                        }  
                    } 
                }
                
                //****************************【 3  设置 ONLY_LEN  [ NEW ]唯一码值】************************************************************************************
                
                taskENTER_CRITICAL();
                
                memset(usart2_only_len_buf,'\0',4);   // 清理，准备保存 pc 设置的唯一码
                memset(cmd_buf,'\0',sizeof(cmd_buf));  
                
                w25q32_flash_read(usart2_only_len_buf,0x00000046,3,2);  // 唯一码读取 (要读取3个字节，目的是把\R\N读取出来)
                if (str_extract_before_first((const char *)usart2_only_len_buf, "\r\n", (char*)cmd_buf, 3)  == true)  // 找到了有效字符 保存在 cmd_buf
                {
                     
                    only_global_pc_set_value = cmd_buf[0] ;  // 将从flash 中读取的唯一码值赋值给全局变量使用
//                    usart1_printf("cmd_buf = %02x\r\n",cmd_buf[0]); 
                    memset(usart2_only_len_buf,'\0',4); 
                    hex_arr_to_str(cmd_buf, 1, (char*)usart2_only_len_buf); // 将十六进制数据，转换为十六进制字符串，保存在 usart2_only_len_buf
                    display_ascii_string(85,261,8,(u8*)"0X",Green,Black); 
                    display_ascii_string(101,261,8,(u8*)usart2_only_len_buf,Green,Black); 

                    pc_reback_cmd[2] = only_global_pc_set_value ;  //  将老的cmd 赋值给返回对比的pc_reback_cmd ，pc_reback_cmd[7]   = {0x7F,0X07,【0X00/cmd】0X80,0X00,0X00,[0X00/fcs]}
                    u8 pc_reback_new_fcs = 0 ;  //  保存上位机返回的异或校验值
                    if (xor_checksum_range(pc_reback_cmd, 1, 5, &pc_reback_new_fcs) == true) // 下位机计算的异或校验值 成功, 并将异或校验值保存在 pc_reback_new_fcs
                    {
                        pc_reback_cmd[6]  = pc_reback_new_fcs ;  //  将下位机自己计算的异或校验值赋值给命令   
                    } 
//                    for (char i = 0 ; i < 7; i++)
//                    {
//                        usart1_printf("%02X--",pc_reback_cmd[i]);
//                    }
                    usart1_printf("ONLY_LEN = 0X%s\r\n",usart2_only_len_buf);
                }
                else
                { 
                    display_ascii_string(85,261,8,(u8*)"NEW_ONLY_LEN SET ERROR",Red,Black); 
                    usart1_printf("NEW_ONLY_LEN SET ERROR\r\n");
                }
                
               taskEXIT_CRITICAL();
               //****************************【 4  设置 ONLY_LEN  [ old 老的 ]唯一码值】****************************************************************************
               
                if(cmd_select_flag == true)  //    使用cmd 老的规则来解析数据
                {
                    memset(usart2_only_len_buf,'\0',4);   // 清理，准备保存 pc 设置的唯一码
                    memset(cmd_buf,'\0',sizeof(cmd_buf));  
                    
                    w25q32_flash_read(usart2_only_len_buf,0x00000049,3,2);  // 唯一码读取 (要读取3个字节，目的是把\R\N读取出来)
                    if (str_extract_before_first((const char *)usart2_only_len_buf, "\r\n", (char*)cmd_buf, 3)  == true)  // 找到了有效字符 保存在 cmd_buf
                    {  
                        lcd_fill_color(5,414,317,426,Black);  // 将display_ascii_string(5,414,6,(u8*)"OLD_ONLY_LEN SET ERROR",Red,Black);  擦除掉
                        old_only_global_pc_set_value = cmd_buf[0] ;  // 将从flash 中读取的唯一码值赋值给全局变量使用
 
                        memset(usart2_only_len_buf,'\0',4); 
                        hex_arr_to_str(cmd_buf, 1, (char*)usart2_only_len_buf); // 将十六进制数据，转换为十六进制字符串，保存在 usart2_only_len_buf
                        display_ascii_string(121,261,8,(u8*)"0X",Green,Black); 
                        display_ascii_string(137,261,8,(u8*)usart2_only_len_buf,Green,Black); 

                        pc_reback_cmd[2] = old_only_global_pc_set_value ;  //  将老的cmd 赋值给返回对比的pc_reback_cmd ，pc_reback_cmd[7]   = {0x7F,0X07,【0X00/cmd】0X80,0X00,0X00,[0X00/fcs]}
                        u8 pc_reback_old_fcs = 0 ;  //  保存上位机返回的异或校验值
                        if (xor_checksum_range(pc_reback_cmd, 1, 5, &pc_reback_old_fcs) == true) // 下位机计算的异或校验值 成功
                        {
                            pc_reback_cmd[6]  = pc_reback_old_fcs ;  //  将下位机自己计算的异或校验值赋值给命令   
                        }
//                        for (char i = 0 ; i < 7; i++)
//                        {
//                            usart1_printf("%02X--",pc_reback_cmd[i]);
//                        }
                        usart1_printf("OLD_ONLY_LEN = 0X%s\r\n",usart2_only_len_buf);
                    }
                    else
                    { 
                        display_ascii_string(5,414,6,(u8*)"OLD_ONLY_LEN SET ERROR",Red,Black); 
                        usart1_printf("OLD_ONLY_LEN SET ERROR\r\n");
                    }
                }
                else // 使用新的cmd 规则来解析数据时，需要将从flash中读取出来的老的cmd 规则擦除，防止误解
                {
                    lcd_fill_color(121,261,317,277,Black); 
                }
                lcd_fill_color(5,414,317,426,Black);  // 将display_ascii_string(5,414,6,(u8*)"OLD_ONLY_LEN SET ERROR",Red,Black);  擦除掉
               //  上电默认显示xor 异或校验方式解析数据，直接确认后，更新为绿色显示
                if(decoding_setting_flag == false)
                {
                    display_ascii_string(165,261,8,(u8*)"CIPHERTEXT",Green,Black); // 密文解析数据
                }
                else
                {
                    display_ascii_string(165,261,8,(u8*)"PLAINTEXT",Green,Black); // 密文解析数据
                }
                
                //********************************【 5 设置开始扫描工作】*********************************************************************************
                if( ( feature_set_ok_from_flash == true ) && ( rssi_set_ok_from_flash == true ) )                 //  当标志为真的时候，表示要设置开始扫描指令
                {
                    cmd_set_flag_from_buttom = false ;
                    feature_set_ok_from_flash = false ;
                    rssi_set_ok_from_flash = false ;
                    usart2_printf("%s","start_scanning\r\n");         //  发送开始扫描命令
                }
                
            }
        }
        

        
        //  用于指示usart2的任务是否在运行
        vTaskDelay(pdMS_TO_TICKS(100));	
        
        taskENTER_CRITICAL();
        end_x = end_x + 2 ;
        if( end_x  <= 318 )
        {
            lcd_fill_color(start_x , 430 , end_x , 436 , Yellow);  // 显示程序运行进度条
            start_x = end_x ;
        }
        else
        {  
           start_x = 3 ;
           end_x = 0 ;
           lcd_fill_color(3,430,318,436,Black);  // 清理掉上次的显示 （进度条）
        }
        taskEXIT_CRITICAL();
        
    }

}

/*******************************************************************
********************************************************************
* 名称：void fixture_start_task(void* pvParameters)
* 功能：fixture_start_task 任务函数
* 参数： 带入任务函数中的参数
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
//void fixture_start_task(void * pvParameters)
//{
////	BaseType_t  task_creak_status;                // 用于保存任务句柄
//	BaseType_t  start_queue_take_back = pdFALSE;  // 用于保存出队函数的返回值
//	u8 fixture_meg_receive = 3;                   // 用于保存接收消息队列的信息
//	
//	while(1)
//	{
//		//消息队列出队获取信息
//		start_queue_take_back = xQueueReceive(fixture_start_queue_handle,(void*)&fixture_meg_receive,0); //portMAX_DELAY 死等
//		if(start_queue_take_back == pdTRUE)
//		{
//			//中断中，治具入队消息为0 ，表示治具下压到位 
//			if(fixture_meg_receive == 0  )
//			{
//				fixture_global_status = true;        // 治具下压状态（用于串口3中，在治具抬起时，串口3才接收数据并判断wifi 复联相关字符）
////				power_ctrl_ext(PWR_3V3_SELECT,true); // 打开 3.3v 电源（模块有可能是3.3v供电）
////				power_ctrl_ext(PWR_5V_SELECT,true);  //打开 5v   电源 （模块有可能是5v供电）
//				//==================================================================================================

//				//===============测试时间计数器=======================================================================
//				time3_count1 = 0; // 清理掉时间计数值，从头开始计数
//				time3_run(1);     // 打开测试定时器3，开始计时
//				lcd_fill_color(13,416,93,432,Tan);  // 清屏上一次测试显示的时间，为接下来的新时间显示做准备
//				display_ascii_string(13,416,8,(u8*)"[00:00:00]",Purple,Tan); // 在打开定时器的时候就显示0，表示开始计时
////				lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
//	
//			}
//			//中断中，治具入队消息为1，表示治具抬起
//			else if(fixture_meg_receive == 1)
//			{
//				//======================抬起治具关闭电源================================
//				fixture_global_status = false; 
//				time3_run(0);     // 关闭定时器
//				time3_count1 = 0; // 清理掉时间计数值，从头开始计数
////				power_ctrl_ext(PWR_3V3_SELECT,false); // 关闭 3.3v 电源（模块有可能是3.3v供电）
////				power_ctrl_ext(PWR_5V_SELECT,false);  // 关闭 5v   电源 （模块有可能是5v供电）

//			}
//		}

//		vTaskDelay(pdMS_TO_TICKS(10));	
//	}
//}

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
//void test_task(void *pvParameters)
//{
////	BaseType_t  task_creak_status;                // 用于保存任务句柄
////	BaseType_t  ads1262_channel_queue_take_back = pdFALSE;  // 用于保存出队函数的返回值
////	u8 ads1262_channel_meg_receive = 0;                   // 用于保存接收消息队列的信息
//	

//	while(1)
//	{
//    }


//}










/*******************************************************************
********************************************************************
* 名称：void vApplicationStackOverflowHook(TaskHandle_t xTask , char*   pcTaskName)
* 功能：堆栈溢出检测钩子函数
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
void vApplicationStackOverflowHook(TaskHandle_t xTask , char*   pcTaskName)
{
	usart1_printf("stack overflow， %s\r\n",pcTaskName);
}
