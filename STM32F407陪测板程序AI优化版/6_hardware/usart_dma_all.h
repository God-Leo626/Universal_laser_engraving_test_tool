#ifndef __USART_DMA_ALL_H__
#define __USART_DMA_ALL_H__

#include "stm32f4xx.h"
#include "usart_ring_buff.h"

/* 默认启用 USART1 (PC 测试命令) 和 USART2 (nRF52832 BLE 协作)。
 * 8N1、DMA 普通模式接收 + IDLE 中断搬入环形队列；任务负责完整帧解析。
 * printf 接口只能用于启动/任务上下文，内部有界格式化并同步发送。
 */
void usart1_init(u32 bound);
void usart1_printf(char *format, ...);
void usart2_init(u32 bound);
void usart2_printf(char *format, ...);
void usart2_disable(void);
void usart2_enable(void);

/* 以下扩展接口仅在 variable_global.h 对应 EN 宏启用时提供实现。 */
void usart3_init(u32 bound);
void usart3_printf(char *format, ...);
void uart4_init(u32 bound);
void uart4_close(void);
void uart4_printf(char *format, ...);
#endif