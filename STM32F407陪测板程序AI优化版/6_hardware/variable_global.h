#ifndef  __VARIABLE_GLOBAL_H__
#define  __VARIABLE_GLOBAL_H__ 
#include "stm32f4xx.h"
#include "usart_ring_buff.h"
//**************************************************************************************
// CPU专用数据段；名称不保证位于CCM，最终地址以链接map为准。
#define CCMRAM  __attribute__ ((section("ccmram")))  // 不能用于DMA；DMA缓冲必须放入链接脚本指定SRAM的.dma_buf段。


//**************************************************************************************
//**************************************************************************************
extern   volatile unsigned char string_buf[100];              // 用于sprintf函数打印字符串的保存
extern  volatile unsigned char mac_buf[12];                   // 用于保存mac
extern  CCMRAM volatile  bool fixture_global_status ;         // 用于保存治具的抬起， 下压状态   FALSE： 抬起   ture ： 下压（串口3中要用且只用不修改）
extern CCMRAM volatile  bool fixture_status_flip_global ;        // 用于保存key2 按键按下后，状态取反（实验室按一次按键开始，在按键一次结束）

//**************************************************************************************
extern volatile u32 time3_count1 ;                  // time3用于测试时间计数 


//**************************************************************************************
#define USART1_TX_LEN  			    1024 *1                  // 定义最大接收字节数 
#define USART1_REC_LEN  			1024 *2                  // 定义最大接收字节数 900
#define EN_USART1			        1		                 // 使能（1）/禁止（0）串口1接收
extern u8 USART1_TX_BUF[USART1_TX_LEN]; 	                 // 发送缓冲,最大字节
extern u8 USART1_RX_BUF[USART1_REC_LEN];	                 // 接收数据缓冲区1
extern CCMRAM volatile bool  USART1_REVEIVE_OK ;                   // 收到数据的标志  false ： 没有收到数据   true ： 收到了数据
extern CCMRAM stRingBuff_usart1  usart1_receive_buffer ;           // 定义一个ringbuffer 的结构体变量,用于串口接收
//===========================================================================
#define USART2_TX_LEN  			500	                     // 定义最大接收字节数 
#define USART2_REC_LEN  	    1024*4	                     // 定义最大接收字节数 
#define EN_USART2 			    1		                     // 使能（1）/禁止（0）串口2接收

extern u8 USART2_TX_BUF[USART2_TX_LEN] ; 	                 //  串口2的发送缓冲
extern u8 USART2_RX_BUF[USART2_REC_LEN] ;	                 //  接收数据缓冲区1
extern CCMRAM volatile bool  USART2_REVEIVE_OK  ;   	     //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
extern CCMRAM stRingBuff_usart1  usart2_receive_buffer ;     //  定义一个ringbuffer 的结构体变量,用于串口接收
//===========================================================================
#define USART3_TX_LEN  			    800             //定义最大接收字节数    ( 用于 dma 串口发送缓冲)
#define USART3_REC_LEN  			RING_BUFFER_LEN  //定义最大接收字节数   （ 用于 dma 串口接收缓冲）
#define EN_USART3 			        0		        //使能（1）/禁止（0）串口3接收
extern u8 USART3_TX_BUF[USART3_TX_LEN] ; 	        //发送缓冲,最大字节
extern u8 USART3_RX_BUF[USART3_REC_LEN] ;	        //接收数据缓冲区1
extern volatile bool  USART3_REVEIVE_OK ;   	    //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
extern CCMRAM  stRingBuff  usart3_receive_buffer ;  // 定义一个ringbuffer 的结构体变量，用于串口接收
extern char copy_ring_buffer[USART3_REC_LEN]  ;     // 用于保存从usart3_receive_buffer 中读取的数据（串口3的环形缓冲中读取数据）
//===========================================================================
#define UART4_TX_LEN  			    4096  +400           //定义最大接收字节数 
#define UART4_REC_LEN  			    USART1_REC_LEN 	 //定义最大接收字节数 
#define EN_UART4 			        0		         //使能（1）/禁止（0）串口4接收

extern u8 UART4_TX_BUF[UART4_TX_LEN]; 	            //发送缓冲,最大字节
extern u8 UART4_RX_BUF[UART4_REC_LEN];	            //接收数据缓冲区1
extern volatile bool  UART4_REVEIVE_OK ;   	                //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
extern stRingBuff_usart1  uart4_receive_buffer ;    // 定义一个ringbuffer 的结构体变量，用于串口接收
extern CCMRAM char uart4_copy_ring_buffer[UART4_REC_LEN] ;        // 用于uart4保存从环形队列中读取到数据
//===========================================================================
#define UART5_TX_LEN  			    10	          //定义最大接收字节数 200
#define UART5_REC_LEN  			    10  	      //定义最大接收字节数 200
#define EN_UART5 			        0		          //使能（1）/禁止（0）串口5接收
//===========================================================================
#define NO_USE_MICRO_LIB            1                // 1: 不使用micro lib   0 ： 使用micro lib




//**************************************************************************************

/*====================================================================*/


extern u8 flash_readbuf[4096] ;
#endif


