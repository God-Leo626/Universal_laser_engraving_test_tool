#ifndef  __USART_DMA_ALL_H__
#define  __USART_DMA_ALL_H__
//**************************************************************************************
#include "stdio.h"
#include "stm32f4xx.h"
#include "stdbool.h"
#include "usart_ring_buff.h"

//**************************************************************************************
void usart1_init(u32 bound);
void usart1_printf(char *format, ...);

void usart2_init(u32 bound);
void usart2_printf(char *format, ...);
void usart2_disable(void);
void usart2_enable(void);

//void usart3_init(u32 bound);
//void usart3_printf(char *format, ...);


//void uart4_init(u32 bound);
//void uart4_close(void);
//void uart4_printf(char *format, ...);

#endif


