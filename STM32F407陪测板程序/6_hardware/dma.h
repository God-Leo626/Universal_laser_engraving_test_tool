#ifndef  __DMA_H__
#define  __DMA_H__
#include "stm32f4xx.h"
//**************************************************************************************
//extern DMA_InitTypeDef  DMA1_tx_Init;      // DMA 接收初始化结构体全局变量        
//extern DMA_InitTypeDef  DMA1_rx_Init;      // DMA 发送初始化结构体全局变量  

//extern DMA_InitTypeDef  DMA1_tx3_Init;      // DMA 接收初始化结构体全局变量        
//extern DMA_InitTypeDef  DMA1_rx3_Init;      // DMA 发送初始化结构体全局变量 

//extern DMA_InitTypeDef  DMA2_tx4_Init;      // DMA 接收初始化结构体全局变量        
//extern DMA_InitTypeDef  DMA2_rx4_Init;      // DMA 发送初始化结构体全局变量 

//**************************************************************************************
void usart1_dma_tx_configuration(void);
void usart1_dma_rx_configuration(void);
void usart1_dma_tx_send(uint8_t *send_buffer , u32 nSendCount);


void usart2_dma_tx_configuration(void);
void usart2_dma_rx_configuration(void);
void usart2_dma_tx_send(uint8_t *send_buffer , u32 nSendCount);

void usart3_dma_tx_configuration(void);
void usart3_dma_rx_configuration(void);
void usart3_dma_tx_send(uint8_t *send_buffer , u32 nSendCount);


void uart4_dma_tx_configuration(void);
void uart4_dma_rx_configuration(void);
void uart4_dma_tx_send(uint8_t *send_buffer , u32 nSendCount);

#endif


