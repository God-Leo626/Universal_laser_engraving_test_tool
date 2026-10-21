#ifndef __DMA_H__
#define __DMA_H__

#include "stm32f4xx.h"
#include <stdarg.h>

/* 发送保持同步：返回时 DMA 已读完发送缓存。有效二进制长度为 1..缓存大小；
 * 长度 0 是空操作。只能由任务/启动代码调用，禁止在 ISR 中发送或格式化。
 * 每个串口共用一把互斥锁，保护“格式化/复制 -> DMA 完成”整个过程。
 */
void usart1_dma_tx_configuration(void);
void usart1_dma_rx_configuration(void);
void usart1_dma_tx_send(uint8_t *send_buffer, u32 nSendCount);
void usart2_dma_tx_configuration(void);
void usart2_dma_rx_configuration(void);
void usart2_dma_tx_send(uint8_t *send_buffer, u32 nSendCount);
void usart3_dma_tx_configuration(void);
void usart3_dma_rx_configuration(void);
void usart3_dma_tx_send(uint8_t *send_buffer, u32 nSendCount);
void uart4_dma_tx_configuration(void);
void uart4_dma_rx_configuration(void);
void uart4_dma_tx_send(uint8_t *send_buffer, u32 nSendCount);

/* 内部格式化入口：超长或格式化失败时整条拒发，避免截断为错误协议帧。 */
void usart_dma_vprintf(USART_TypeDef *usart, const char *format, va_list args);
/* 调试统计：非法参数、锁不可用、DMA 错误或长时间无进展均累计一次。 */
u32 usart_dma_get_tx_error_count(USART_TypeDef *usart);
#endif