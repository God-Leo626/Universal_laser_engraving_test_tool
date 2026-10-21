#include "my_printf.h"
#include "uarte_52832_dma.h"
#include "nrf_delay.h"
#include <stdarg.h>
#include <stdio.h>

void print(const char *format, ...)
{
    char buffer[PRINT_BUF_LEN];
    va_list args;
    int count;
    uint32_t wait_steps = 5000u;
    if (format == NULL || __get_IPSR() != 0) return;
    va_start(args, format);
    count = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (count <= 0) return;
    if ((size_t)count >= sizeof(buffer)) count = (int)sizeof(buffer) - 1;
    /* 驱动复制到自有 RAM，超时返回也不会让 DMA 使用失效栈地址。 */
    if (uart_dma_send((const uint8_t *)buffer, (uint16_t)count) != UART_DMA_OK) return;
    while (uart_dma_is_tx_busy() && wait_steps-- > 0u) nrf_delay_us(10);
}
