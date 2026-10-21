#include "variable_global.h"
#include "stm32f4xx.h"
#include "usart_ring_buff.h"
//**************************************************************************************
CCMRAM volatile unsigned char string_buf[100]   =  {0};      // 用于sprintf函数打印字符串的保存
CCMRAM volatile unsigned char mac_buf[12]      =   {0};      // 用于保存测试控制板的mac
CCMRAM volatile  bool fixture_global_status = false ;        // 用于保存治具的抬起， 下压状态   FALSE： 抬起   ture ： 下压
CCMRAM volatile  bool fixture_status_flip_global = false ;   // 用于保存key2 按键按下后，状态取反（实验室按一次按键开始，在按键一次结束）

//***************** *********************************************************************
CCMRAM volatile u32 time3_count1    = 0;                    // 用于测试时间计数 
u8  flash_readbuf[4096] = {0};                              // 用于保存从服务器数据中，截取后的数据
//**************************************************************************************
//  DMA 缓冲区统一放入一个自定义段 .dma_buf,且在sct文件中，要指定放入sram短，要不dma访问不了
u8 USART1_TX_BUF[USART1_TX_LEN]   __attribute__((section(".dma_buf"))) = {0}; 	                //串口1的发送缓冲
u8 USART1_RX_BUF[USART1_REC_LEN]  __attribute__((section(".dma_buf"))) = {0};	                //接收数据缓冲区1
CCMRAM volatile bool  USART1_REVEIVE_OK = false ;   	//  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
CCMRAM stRingBuff_usart1  usart1_receive_buffer ;       // 定义一个ringbuffer 的结构体变量,用于串口接收



u8 USART2_TX_BUF[USART2_TX_LEN]   __attribute__((section(".dma_buf")))  = {0}; 	                //串口1的发送缓冲
u8 USART2_RX_BUF[USART2_REC_LEN]  __attribute__((section(".dma_buf")))  = {0};	                //接收数据缓冲区1
CCMRAM volatile bool  USART2_REVEIVE_OK = false ;   	//  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
CCMRAM stRingBuff_usart1  usart2_receive_buffer ;       // 定义一个ringbuffer 的结构体变量,用于串口接收



u8 USART3_TX_BUF[USART3_TX_LEN]   __attribute__((section(".dma_buf"))) = {0}; 	                //发送缓冲,最大字节
u8 USART3_RX_BUF[USART3_REC_LEN]  __attribute__((section(".dma_buf"))) = {0};	                //接收数据缓冲区 （用于串口3的dma的接收保存数据）
CCMRAM volatile bool  USART3_REVEIVE_OK = false ;   	//  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
CCMRAM  stRingBuff  usart3_receive_buffer ;             // 定义一个ringbuffer 的结构体变量，用于串口接收
char copy_ring_buffer[USART3_REC_LEN] = {0};	 ;      // 用于保存从usart3_receive_buffer 中读取的数据（串口3的环形缓冲中读取数据）


u8 UART4_TX_BUF[UART4_TX_LEN]    __attribute__((section(".dma_buf"))) = {0}; 	             //发送缓冲,最大字节
u8 UART4_RX_BUF[UART4_REC_LEN]   __attribute__((section(".dma_buf"))) = {0};	             //接收数据缓冲区1
bool  UART4_REVEIVE_OK = false ;   	                 //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
CCMRAM stRingBuff_usart1  uart4_receive_buffer ;     // 定义一个ringbuffer 的结构体变量，用于串口接收
CCMRAM char uart4_copy_ring_buffer[UART4_REC_LEN]  = {0};        // 用于uart4保存从环形队列中读取到数据

//**************************************************************************************
//CCMRAM char token_global_buf[100] = {0};                         // 保存token的全局变量
//CCMRAM char sha256_global_buf[65] = {0};                         // 保存服务器发送的sha256的值  字符长度64
//CCMRAM char original_sha256_global_buf[65] = {0};                // 保存服务器发送的无压缩时bin文件的 sha256的值  字符长度64
//CCMRAM char path_global_buf[37] = {0};                           // 保存固件下载的路径信息  路径字符36
//CCMRAM char license_global_buf[120] = {0};                       // 保存云端下载的602 的license全部数据
//CCMRAM char license_device_id_global_buf[11] = {0};              // 保存云端下载的602 的license数据 中的 device id
//CCMRAM char bin_length_global_buf[10] = {0};                     // 保存压缩bin文件的长度
//CCMRAM char original_bin_length_global_buf[10] = {0};            // 保存原bin文件的长度
//CCMRAM volatile bool  test_global_flag  =  false;                // 测试模式标志 false ： 烧录，测试，激活一体   true ： 只测试模式
//CCMRAM volatile bool  worknumber_config_global_flag = false ;    //  true : 表示在工单配置模式下   false ：表示不在工单配置模式下

//**************************************************************************************
//CCMRAM char current_A_string_buf[16] = {0};    // 保存零点校准时的电流A的字符串形式
/*====================================================================*/





