#include "dma.h"
#include "variable_global.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* 数据流/通道保持原接线；所有 USART 均使用 Channel 4。
 * USART1 TX/RX = DMA2 Stream7/2；USART2 = DMA1 Stream6/5；
 * USART3 TX/RX = DMA1 Stream3/1；UART4  = DMA1 Stream4/2。
 * DMA 缓存在 variable_global.c 的 .dma_buf (SRAM)，不能放在 CCMRAM。
 */
typedef struct
{
    USART_TypeDef *usart;
    DMA_Stream_TypeDef *stream;
    uint8_t *buffer;
    u32 capacity;
    u32 complete_flag;
    u32 transfer_error_flag;
    u32 direct_error_flag;
    u32 fifo_error_flag;
    u32 clear_flags;
    SemaphoreHandle_t mutex;
    volatile u32 errors;
} UartDmaTx;

static UartDmaTx tx_ports[] =
{
    { USART1, DMA2_Stream7, USART1_TX_BUF, (USART1_TX_LEN), DMA_FLAG_TCIF7,
      DMA_FLAG_TEIF7, DMA_FLAG_DMEIF7, DMA_FLAG_FEIF7,
      DMA_FLAG_TCIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_FEIF7, NULL, 0U },
    { USART2, DMA1_Stream6, USART2_TX_BUF, (USART2_TX_LEN), DMA_FLAG_TCIF6,
      DMA_FLAG_TEIF6, DMA_FLAG_DMEIF6, DMA_FLAG_FEIF6,
      DMA_FLAG_TCIF6 | DMA_FLAG_HTIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_FEIF6, NULL, 0U },
    { USART3, DMA1_Stream3, USART3_TX_BUF, (USART3_TX_LEN), DMA_FLAG_TCIF3,
      DMA_FLAG_TEIF3, DMA_FLAG_DMEIF3, DMA_FLAG_FEIF3,
      DMA_FLAG_TCIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_FEIF3, NULL, 0U },
    { UART4, DMA1_Stream4, UART4_TX_BUF, (UART4_TX_LEN), DMA_FLAG_TCIF4,
      DMA_FLAG_TEIF4, DMA_FLAG_DMEIF4, DMA_FLAG_FEIF4,
      DMA_FLAG_TCIF4 | DMA_FLAG_HTIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_FEIF4, NULL, 0U }
};

/* 这是无进展轮询次数上限，不依赖 SysTick，启动阶段也可退出故障等待。
 * 每当 NDTR 减小就重置预算，不限制合法长报文的总发送时间。
 */
#define DMA_DISABLE_POLL_LIMIT 100000U
#define DMA_STALL_POLL_LIMIT  1000000U

static bool dma_stop(DMA_Stream_TypeDef *stream)
{
    u32 budget = DMA_DISABLE_POLL_LIMIT;
    DMA_Cmd(stream, DISABLE);
    while (DMA_GetCmdStatus(stream) != DISABLE)
        if (--budget == 0U) return false;
    return true;
}

static void tx_error(UartDmaTx *port)
{
    uint32_t saved = __get_PRIMASK();
    __disable_irq();
    ++port->errors;
    __set_PRIMASK(saved);
}

static UartDmaTx *find_tx_port(USART_TypeDef *usart)
{
    unsigned int i;
    for (i = 0U; i < sizeof(tx_ports) / sizeof(tx_ports[0]); ++i)
        if (tx_ports[i].usart == usart) return &tx_ports[i];
    return NULL;
}

/* 初始化时创建一次互斥锁。启动阶段/已有临界区中只尝试获取，不阻塞调度器。
 * IRQ 只接收数据和置标志，不能调用任务互斥量或长时间等待串口。
 */
static bool tx_lock(UartDmaTx *port)
{
    TickType_t wait = 0U;
    if (__get_IPSR() != 0U || port->mutex == NULL)
    {
        tx_error(port);
        return false;
    }
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING &&
        __get_PRIMASK() == 0U && __get_BASEPRI() == 0U)
        wait = portMAX_DELAY;
    if (xSemaphoreTake(port->mutex, wait) != pdTRUE)
    {
        tx_error(port);
        return false;
    }
    return true;
}

static bool tx_send_locked(UartDmaTx *port, const uint8_t *data, u32 count)
{
    u32 remaining;
    u32 last_remaining = count;
    u32 budget = DMA_STALL_POLL_LIMIT;
    bool success = true;

    if (count == 0U) return true;
    if (data == NULL || count > port->capacity ||
        (port->usart->CR1 & (USART_CR1_UE | USART_CR1_TE)) != (USART_CR1_UE | USART_CR1_TE) ||
        (port->usart->CR3 & USART_CR3_DMAT) == 0U)
        return false;
    if (!dma_stop(port->stream)) return false;

    /* 重发前清除旧标志，避免上次 TC 导致本次误判完成；允许输入即 TX_BUF。 */
    DMA_ClearFlag(port->stream, port->clear_flags);
    if (data != port->buffer) memmove(port->buffer, data, count);
    port->stream->M0AR = (uint32_t)port->buffer;
    DMA_SetCurrDataCounter(port->stream, (uint16_t)count);
    DMA_Cmd(port->stream, ENABLE);

    while (DMA_GetFlagStatus(port->stream, port->complete_flag) == RESET)
    {
        if (DMA_GetFlagStatus(port->stream, port->transfer_error_flag) != RESET ||
            DMA_GetFlagStatus(port->stream, port->direct_error_flag) != RESET ||
            DMA_GetFlagStatus(port->stream, port->fifo_error_flag) != RESET)
        {
            success = false;
            break;
        }
        remaining = DMA_GetCurrDataCounter(port->stream);
        if (remaining != last_remaining)
        {
            last_remaining = remaining;
            budget = DMA_STALL_POLL_LIMIT;
        }
        else if (--budget == 0U)
        {
            success = false;
            break;
        }
    }
    if (DMA_GetFlagStatus(port->stream, port->transfer_error_flag) != RESET ||
        DMA_GetFlagStatus(port->stream, port->direct_error_flag) != RESET ||
        DMA_GetFlagStatus(port->stream, port->fifo_error_flag) != RESET)
        success = false;
    if (!dma_stop(port->stream)) return false;
    DMA_SetCurrDataCounter(port->stream, 0U);
    DMA_ClearFlag(port->stream, port->clear_flags);
    return success;
}

static void tx_send(UartDmaTx *port, const uint8_t *data, u32 count)
{
    if (count == 0U) return;
    if (!tx_lock(port)) return;
    if (!tx_send_locked(port, data, count)) tx_error(port);
    (void)xSemaphoreGive(port->mutex);
}

void usart_dma_vprintf(USART_TypeDef *usart, const char *format, va_list args)
{
    int count;
    UartDmaTx *port = find_tx_port(usart);
    if (port == NULL) return;
    if (format == NULL) { tx_error(port); return; }
    if (!tx_lock(port)) return;
    /* 若上次 DMA 异常未能停止，绝不能提前覆盖它仍在读取的缓冲区。 */
    if (!dma_stop(port->stream))
    {
        tx_error(port);
        (void)xSemaphoreGive(port->mutex);
        return;
    }
    count = vsnprintf((char *)port->buffer, port->capacity, format, args);
    if (count < 0 || (u32)count >= port->capacity)
        tx_error(port);
    else if (!tx_send_locked(port, port->buffer, (u32)count))
        tx_error(port);
    (void)xSemaphoreGive(port->mutex);
}

u32 usart_dma_get_tx_error_count(USART_TypeDef *usart)
{
    UartDmaTx *port = find_tx_port(usart);
    return port != NULL ? port->errors : 0U;
}

/* 统一字节 DMA 配置，方向决定优先级及是否立即启动。
 * TX 的实际 NDTR 在每次发送时设置，初始化使用合法占位值 1。
 */
static void dma_configure(DMA_Stream_TypeDef *stream, u32 dma_clock,
                          USART_TypeDef *usart, uint8_t *buffer,
                          u32 count, u32 direction)
{
    DMA_InitTypeDef init;
    RCC_AHB1PeriphClockCmd(dma_clock, ENABLE);
    if (!dma_stop(stream)) return;
    DMA_DeInit(stream);
    DMA_StructInit(&init);
    init.DMA_Channel = DMA_Channel_4;
    init.DMA_PeripheralBaseAddr = (uint32_t)&usart->DR;
    init.DMA_Memory0BaseAddr = (uint32_t)buffer;
    init.DMA_DIR = direction;
    init.DMA_BufferSize = count;
    init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    init.DMA_MemoryInc = DMA_MemoryInc_Enable;
    init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    init.DMA_Mode = DMA_Mode_Normal;
    init.DMA_Priority = direction == DMA_DIR_PeripheralToMemory ? DMA_Priority_High : DMA_Priority_Medium;
    init.DMA_FIFOMode = DMA_FIFOMode_Disable;
    init.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
    init.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    init.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(stream, &init);
    if (direction == DMA_DIR_PeripheralToMemory) DMA_Cmd(stream, ENABLE);
}

static void tx_configure(UartDmaTx *port, u32 dma_clock)
{
    /* 硬件初始化阶段调用一次，失败后发送接口会累计错误并拒绝使用共享缓存。 */
    if (port->mutex == NULL) port->mutex = xSemaphoreCreateMutex();
    dma_configure(port->stream, dma_clock, port->usart, port->buffer, 1U, DMA_DIR_MemoryToPeripheral);
}

void usart1_dma_tx_configuration(void) { tx_configure(&tx_ports[0], RCC_AHB1Periph_DMA2); }
void usart2_dma_tx_configuration(void) { tx_configure(&tx_ports[1], RCC_AHB1Periph_DMA1); }
void usart3_dma_tx_configuration(void) { tx_configure(&tx_ports[2], RCC_AHB1Periph_DMA1); }
void uart4_dma_tx_configuration(void)  { tx_configure(&tx_ports[3], RCC_AHB1Periph_DMA1); }

void usart1_dma_rx_configuration(void)
{
    dma_configure(DMA2_Stream2, RCC_AHB1Periph_DMA2, USART1, USART1_RX_BUF, (USART1_REC_LEN), DMA_DIR_PeripheralToMemory);
}
void usart2_dma_rx_configuration(void)
{
    dma_configure(DMA1_Stream5, RCC_AHB1Periph_DMA1, USART2, USART2_RX_BUF, (USART2_REC_LEN), DMA_DIR_PeripheralToMemory);
}
void usart3_dma_rx_configuration(void)
{
    dma_configure(DMA1_Stream1, RCC_AHB1Periph_DMA1, USART3, USART3_RX_BUF, (USART3_REC_LEN), DMA_DIR_PeripheralToMemory);
}
void uart4_dma_rx_configuration(void)
{
    dma_configure(DMA1_Stream2, RCC_AHB1Periph_DMA1, UART4, UART4_RX_BUF, (UART4_REC_LEN), DMA_DIR_PeripheralToMemory);
}

void usart1_dma_tx_send(uint8_t *data, u32 count) { tx_send(&tx_ports[0], data, count); }
void usart2_dma_tx_send(uint8_t *data, u32 count) { tx_send(&tx_ports[1], data, count); }
void usart3_dma_tx_send(uint8_t *data, u32 count) { tx_send(&tx_ports[2], data, count); }
void uart4_dma_tx_send(uint8_t *data, u32 count)  { tx_send(&tx_ports[3], data, count); }