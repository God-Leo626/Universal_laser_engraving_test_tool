#ifndef  __DMA_H__
#define  __DMA_H__
#include "stm32f10x.h"
//**************************************************************************************
extern DMA_InitTypeDef  DMA1_tx_Init;      // DMA 接收初始化结构体全局变量        
extern DMA_InitTypeDef  DMA1_rx_Init;      // DMA 发送初始化结构体全局变量  

extern DMA_InitTypeDef  DMA1_tx3_Init;      // DMA 接收初始化结构体全局变量        
extern DMA_InitTypeDef  DMA1_rx3_Init;      // DMA 发送初始化结构体全局变量 

extern DMA_InitTypeDef  DMA2_tx4_Init;      // DMA 接收初始化结构体全局变量        
extern DMA_InitTypeDef  DMA2_rx4_Init;      // DMA 发送初始化结构体全局变量 
//**************************************************************************************

void dma1_usart1_init(void);
void dma1_usart3_init(void);
void dma2_uart4_init(void);

#endif


