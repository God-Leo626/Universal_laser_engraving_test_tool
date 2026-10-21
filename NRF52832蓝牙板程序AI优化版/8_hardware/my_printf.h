#ifndef MY_PRINTF_H
#define MY_PRINTF_H
#define PRINT_BUF_LEN 256u
/* 仅主循环调用：每次最多格式化 255 字节，等待发送最多约 50 ms。
 * 超长格式化输出会截断；传输繁忙或失败时丢弃本次输出，不覆盖正在发送的数据。
 */
void print(const char *format, ...);
#endif
