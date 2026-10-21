#ifndef __USART_RING_BUFF_H__
#define __USART_RING_BUFF_H__

#include "stm32f4xx.h"
#include <stdbool.h>

#define RING_BUFFER_LEN          1024U
#define USART1_RING_BUFFER_LEN   1024U
#define UART4_RING_BUFFER_LEN     500U

/* 中断写入、任务读取；索引和长度由实现中的短临界区保护。
 * dropped 累计软件溢出字节数及硬件接收错误事件，初始化后从零计数。
 * 硬件错误实际丢字节数未知；此值仅用于检测字节流断裂并重新寻找帧边界。
 */
typedef struct ringBuff
{
    volatile unsigned int in;
    volatile unsigned int out;
    volatile unsigned int length;
    volatile unsigned int dropped;
    unsigned char buffer[RING_BUFFER_LEN];
} stRingBuff;

typedef struct ringBuff_usart1
{
    volatile unsigned int in;
    volatile unsigned int out;
    volatile unsigned int length;
    volatile unsigned int dropped;
    unsigned char buffer[USART1_RING_BUFFER_LEN];
} stRingBuff_usart1;

void ring_buffer_init(stRingBuff *ringBuf);
bool is_ring_buffer_full(stRingBuff *ringBuf);
bool is_ringbuffer_empty(stRingBuff *ringBuf);
bool write_onebyte_to_ringbuffer(stRingBuff *ringBuf, char data);
bool read_one_byte_from_ringbuffer(stRingBuff *ringBuf, char *data);
void write_ring_bufer(stRingBuff *ringBuf, char *writeBuf, unsigned int len);
void read_ringbuffer(stRingBuff *ringBuf, char *readBuf, unsigned int len);
unsigned int get_ringbuffer_length(stRingBuff *ringBuf);
unsigned int get_ringbuffer_dropped(stRingBuff *ringBuf);
unsigned int read_ringbuffer_count(stRingBuff *ringBuf, char *readBuf, unsigned int max_len);

/* USART1/USART2/UART4 沿用此接口；读取最多 max_len 个字节并返回实际长度。
 * 二进制读取不追加 '\0'；如需字符串，调用者预留一个字节并自行终止。
 * 旧 void 读取接口保留兼容，但新代码应使用带 count 的接口。
 */
void ring_buffer_init_for_usart1(stRingBuff_usart1 *ringBuf);
bool is_ring_buffer_full_for_usart1(stRingBuff_usart1 *ringBuf);
bool is_ringbuffer_empty_for_usart1(stRingBuff_usart1 *ringBuf);
bool write_onebyte_to_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf, char data);
bool read_one_byte_from_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf, char *data);
void write_ring_bufer_for_usart1(stRingBuff_usart1 *ringBuf, char *writeBuf, unsigned int len);
void read_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf, char *readBuf, unsigned int len);
unsigned int get_ringbuffer_length_for_usart1(stRingBuff_usart1 *ringBuf);
unsigned int get_ringbuffer_dropped_for_usart1(stRingBuff_usart1 *ringBuf);
unsigned int read_ringbuffer_count_for_usart1(stRingBuff_usart1 *ringBuf, char *readBuf, unsigned int max_len);
#endif