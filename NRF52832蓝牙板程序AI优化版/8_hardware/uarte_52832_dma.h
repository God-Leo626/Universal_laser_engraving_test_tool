#ifndef UARTE_52832_DMA_H
#define UARTE_52832_DMA_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define RX_PIN_NUMBER 8u
#define TX_PIN_NUMBER 6u
#define UART_RX_BUF_SIZE 255u       /* nRF52832 EasyDMA MAXCNT 为 8 位。 */
#define UART_TX_BUF_SIZE 255u
#define UART_RX_TIMEOUT_US 2000u

typedef enum {
    UART_DMA_OK = 0,
    UART_DMA_ERR_BUSY,
    UART_DMA_ERR_INVALID_PARAM,
    UART_DMA_ERR_INVALID_STATE
} uart_dma_err_t;

void uarte_dma_init(void);
/* 仅主循环调用；复制到驱动自有 RAM 缓冲，返回后调用方可复用输入。 */
uart_dma_err_t uart_dma_send(const uint8_t *data, uint16_t length);
bool uart_dma_is_tx_busy(void);
/* 仅主循环调用。取出已停止的 DMA 数据，并在返回前重启接收。
 * damaged=true 表示硬件错误，此次数据不得作为命令执行。
 * capacity 必须 >= UART_RX_BUF_SIZE；无完整块时不修改输出参数。
 */
bool uart_dma_read(uint8_t *data, size_t capacity, size_t *length, bool *damaged);
#endif
