#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H
#include <stddef.h>
#include <stdint.h>

#define UART_COMMAND_MAX_LENGTH 254u

/* 主循环调用；运行阶段仅释放接收缓冲，不再修改配置。 */
void uart_data_processing(void);
/* 按字节拼接 CRLF 命令；支持一次接收多行以及一行跨多次接收。 */
void uart_commands_feed(const uint8_t *data, size_t length);
/* 接收发生硬件错误后，丢到下一行末，避免执行残缺命令的尾部。 */
void uart_commands_discard_partial(void);
#endif
