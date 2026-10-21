#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FEATURE_RAW_CAPACITY 31u
#define ENCRYPTED_PACKET_SIZE 31u
#define DECRYPTED_DATA_SIZE 21u
#define DECRYPTED_BUFFER_SIZE 22u

/* 仅配置阶段修改；feature_set 失败时保留上一次成功设置。 */
extern uint8_t feature_raw[FEATURE_RAW_CAPACITY];
extern uint8_t feature_raw_len;
extern bool feature_global_flag;

/* 接受 1~31 字节的纯十六进制字符串，不能长于传统 BLE 广播。 */
bool feature_set(const char *hex_str);
/* 只有 *ok 为 true，返回数值才有效；拒绝空串、溢出和尾随字符。 */
long num_str_to_num(const char *text, bool *ok, uint8_t base);
/* dst_size 包含字符串末尾的 NUL；失败时输出空串。 */
bool str_extract(const char *src, const char *start, const char *end, char *dst, size_t dst_size);
/* 固定 31 字节输入，输出容量至少 22 字节；失败时不写输出。输入输出不得重叠。 */
bool decrypt_data(const uint8_t *data, size_t length, uint8_t *output, int output_size);
#endif
