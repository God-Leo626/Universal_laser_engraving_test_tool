#include "usart_ring_buff.h"
#include <string.h>

/* USART 中断优先级高于 FreeRTOS 的 BASEPRI 屏蔽范围，故这里保存/恢复
 * PRIMASK，而非 taskENTER_CRITICAL。仅一个字节的索引和长度更新需要互斥；
 * 可从任务或中断调用，退出时保留调用前的中断开关状态。
 */
static uint32_t ring_lock(void)
{
    uint32_t saved = __get_PRIMASK();
    __disable_irq();
    return saved;
}

static void ring_unlock(uint32_t saved)
{
    __set_PRIMASK(saved);
}
/* USART3 通用队列：只清空本结构体自己的数组，不能使用 DMA 接收区长度。 */
void ring_buffer_init(stRingBuff *ringBuf)
{
    uint32_t saved;
    if (ringBuf == NULL) return;
    saved = ring_lock();
    ringBuf->in = 0U;
    ringBuf->out = 0U;
    ringBuf->length = 0U;
    ringBuf->dropped = 0U;
    memset(ringBuf->buffer, 0, sizeof(ringBuf->buffer));
    ring_unlock(saved);
}

bool is_ring_buffer_full(stRingBuff *ringBuf)
{
    return ringBuf != NULL && ringBuf->length >= RING_BUFFER_LEN;
}

bool is_ringbuffer_empty(stRingBuff *ringBuf)
{
    return ringBuf == NULL || ringBuf->length == 0U;
}

bool write_onebyte_to_ringbuffer(stRingBuff *ringBuf, char data)
{
    uint32_t saved;
    if (ringBuf == NULL) return false;
    saved = ring_lock();
    if (ringBuf->length >= RING_BUFFER_LEN)
    {
        ++ringBuf->dropped;
        ring_unlock(saved);
        return false;
    }
    ringBuf->buffer[ringBuf->in] = (unsigned char)data;
    ringBuf->in = (ringBuf->in + 1U) % RING_BUFFER_LEN;
    ++ringBuf->length;
    ring_unlock(saved);
    return true;
}

bool read_one_byte_from_ringbuffer(stRingBuff *ringBuf, char *data)
{
    uint32_t saved;
    if (ringBuf == NULL || data == NULL) return false;
    saved = ring_lock();
    if (ringBuf->length == 0U)
    {
        ring_unlock(saved);
        return false;
    }
    *data = (char)ringBuf->buffer[ringBuf->out];
    ringBuf->out = (ringBuf->out + 1U) % RING_BUFFER_LEN;
    --ringBuf->length;
    ring_unlock(saved);
    return true;
}

void write_ring_bufer(stRingBuff *ringBuf, char *writeBuf, unsigned int len)
{
    unsigned int i;
    if (writeBuf == NULL) return;
    for (i = 0U; i < len; ++i)
        (void)write_onebyte_to_ringbuffer(ringBuf, writeBuf[i]);
}

unsigned int read_ringbuffer_count(stRingBuff *ringBuf, char *readBuf,
                                           unsigned int max_len)
{
    unsigned int count = 0U;
    if (readBuf == NULL) return 0U;
    while (count < max_len && read_one_byte_from_ringbuffer(ringBuf, &readBuf[count]))
        ++count;
    return count;
}

void read_ringbuffer(stRingBuff *ringBuf, char *readBuf, unsigned int len)
{
    (void)read_ringbuffer_count(ringBuf, readBuf, len);
}

unsigned int get_ringbuffer_length(stRingBuff *ringBuf)
{
    return ringBuf != NULL ? ringBuf->length : 0U;
}

unsigned int get_ringbuffer_dropped(stRingBuff *ringBuf)
{
    return ringBuf != NULL ? ringBuf->dropped : 0U;
}

/* USART1/USART2/UART4 队列：只清空本结构体自己的数组，不能使用 DMA 接收区长度。 */
void ring_buffer_init_for_usart1(stRingBuff_usart1 *ringBuf)
{
    uint32_t saved;
    if (ringBuf == NULL) return;
    saved = ring_lock();
    ringBuf->in = 0U;
    ringBuf->out = 0U;
    ringBuf->length = 0U;
    ringBuf->dropped = 0U;
    memset(ringBuf->buffer, 0, sizeof(ringBuf->buffer));
    ring_unlock(saved);
}

bool is_ring_buffer_full_for_usart1(stRingBuff_usart1 *ringBuf)
{
    return ringBuf != NULL && ringBuf->length >= USART1_RING_BUFFER_LEN;
}

bool is_ringbuffer_empty_for_usart1(stRingBuff_usart1 *ringBuf)
{
    return ringBuf == NULL || ringBuf->length == 0U;
}

bool write_onebyte_to_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf, char data)
{
    uint32_t saved;
    if (ringBuf == NULL) return false;
    saved = ring_lock();
    if (ringBuf->length >= USART1_RING_BUFFER_LEN)
    {
        ++ringBuf->dropped;
        ring_unlock(saved);
        return false;
    }
    ringBuf->buffer[ringBuf->in] = (unsigned char)data;
    ringBuf->in = (ringBuf->in + 1U) % USART1_RING_BUFFER_LEN;
    ++ringBuf->length;
    ring_unlock(saved);
    return true;
}

bool read_one_byte_from_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf, char *data)
{
    uint32_t saved;
    if (ringBuf == NULL || data == NULL) return false;
    saved = ring_lock();
    if (ringBuf->length == 0U)
    {
        ring_unlock(saved);
        return false;
    }
    *data = (char)ringBuf->buffer[ringBuf->out];
    ringBuf->out = (ringBuf->out + 1U) % USART1_RING_BUFFER_LEN;
    --ringBuf->length;
    ring_unlock(saved);
    return true;
}

void write_ring_bufer_for_usart1(stRingBuff_usart1 *ringBuf, char *writeBuf, unsigned int len)
{
    unsigned int i;
    if (writeBuf == NULL) return;
    for (i = 0U; i < len; ++i)
        (void)write_onebyte_to_ringbuffer_for_usart1(ringBuf, writeBuf[i]);
}

unsigned int read_ringbuffer_count_for_usart1(stRingBuff_usart1 *ringBuf, char *readBuf,
                                           unsigned int max_len)
{
    unsigned int count = 0U;
    if (readBuf == NULL) return 0U;
    while (count < max_len && read_one_byte_from_ringbuffer_for_usart1(ringBuf, &readBuf[count]))
        ++count;
    return count;
}

void read_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf, char *readBuf, unsigned int len)
{
    (void)read_ringbuffer_count_for_usart1(ringBuf, readBuf, len);
}

unsigned int get_ringbuffer_length_for_usart1(stRingBuff_usart1 *ringBuf)
{
    return ringBuf != NULL ? ringBuf->length : 0U;
}

unsigned int get_ringbuffer_dropped_for_usart1(stRingBuff_usart1 *ringBuf)
{
    return ringBuf != NULL ? ringBuf->dropped : 0U;
}
