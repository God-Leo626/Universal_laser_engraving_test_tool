#ifndef  __USART_DMA_ALL_H__
#define  __USART_DMA_ALL_H__
//**************************************************************************************
#include "stdio.h"
#include "stm32f10x.h"
#include "stdbool.h"
#include "usart_ring_buff.h"
//===========================================================================
#define USART1_TX_LEN  			    400         //定义最大接收字节数 200
#define USART1_REC_LEN  			400         //定义最大接收字节数 200
#define EN_USART1			        1		     //使能（1）/禁止（0）串口1接收
//===========================================================================
//#define USART2_TX_LEN  			    10	         //定义最大接收字节数 200
//#define USART2_REC_LEN  			10	         //定义最大接收字节数 200
//#define EN_USART2 			        0		     //使能（1）/禁止（0）串口2接收
//===========================================================================
#define USART3_TX_LEN  			    800         //定义最大接收字节数    ( 用于 dma 串口发送缓冲)
#define USART3_REC_LEN  			10     //定义最大接收字节数   （ 用于 dma 串口接收缓冲）
#define EN_USART3 			        1		     //使能（1）/禁止（0）串口3接收
//===========================================================================
#define UART4_TX_LEN  			    100            //定义最大接收字节数 200
#define UART4_REC_LEN  			    10 	      //定义最大接收字节数 200
#define EN_UART4 			        1		      //使能（1）/禁止（0）串口4接收
//===========================================================================
//#define UART5_TX_LEN  			    10	          //定义最大接收字节数 200
//#define UART5_REC_LEN  			    10  	      //定义最大接收字节数 200
//#define EN_UART5 			        0		      //使能（1）/禁止（0）串口5接收
//===========================================================================
#define NO_USE_MICRO_LIB            0             // 1: 不使用micro lib   0 ： 使用micro lib


extern u8 USART1_TX_BUF[USART1_TX_LEN]; 	    // 发送缓冲,最大字节
extern u8 USART1_RX_BUF_1[USART1_REC_LEN];	    // 接收数据缓冲区1
//extern u8 USART1_RX_BUF_2[USART1_REC_LEN];	    // 接收数据缓冲区2
//extern bool USART1_REVEIVE_OK ;   	            // 收到数据的标志  false ： 没有收到数据   true ： 收到了数据
//extern stRingBuff  usart1_receive_buffer ;      // 定义一个ringbuffer 的结构体变量
extern stRingBuff_usart1  usart1_receive_buffer ;             // 定义一个ringbuffer 的结构体变量,用于串口接收




//extern u8 USART2_TX_BUF[USART2_TX_LEN]; 	    // 发送缓冲,最大字节
//extern u8 USART2_RX_BUF_1[USART2_REC_LEN];	    // 接收数据缓冲区1
//extern u8 USART2_RX_BUF_2[USART2_REC_LEN];	    // 接收数据缓冲区2



extern u8 USART3_TX_BUF[USART3_TX_LEN] ; 	        //发送缓冲,最大字节
extern u8 USART3_RX_BUF_1[USART3_REC_LEN] ;	        //接收数据缓冲区1
//extern u8 USART3_RX_BUF_2[USART3_REC_LEN] ;	        //接收数据缓冲区2
extern volatile bool  USART3_REVEIVE_OK ;   	                //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
extern stRingBuff  usart3_receive_buffer ;          // 定义一个ringbuffer 的结构体变量，用于串口接收



extern u8 UART4_TX_BUF[UART4_TX_LEN]; 	        //发送缓冲,最大字节
extern u8 UART4_RX_BUF_1[UART4_REC_LEN];	    //接收数据缓冲区1
//extern u8 UART4_RX_BUF_2[UART4_REC_LEN];	    //接收数据缓冲区2
extern bool  UART4_REVEIVE_OK ;   	                //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
extern stRingBuff  uart4_receive_buffer ;          // 定义一个ringbuffer 的结构体变量，用于串口接收



//extern u8 UART5_TX_BUF[UART5_TX_LEN]; 	        //发送缓冲,最大字节
//extern u8 UART5_RX_BUF_1[UART5_REC_LEN];	    //接收数据缓冲区1
//extern u8 UART5_RX_BUF_2[UART5_REC_LEN];	    //接收数据缓冲区2


//extern bool usart1_witch_buf ;                  //标记当前使用的是哪个缓冲区,0：使用u1rxbuf；1：使用u2rxbuf
//extern bool usart3_witch_buf ;  


//**************************************************************************************
void usart1_init(u32 bound);
void usart1_printf(char *format, ...);

void usart3_init(u32 bound);
void usart3_printf(char *format, ...);
void u3_printf(char* fmt,...)  ;

void uart4_init(u32 bound);
void uart4_printf(char *format, ...);

#endif


