#ifndef __DATA_PROCESSING_H__
#define __DATA_PROCESSING_H__

#include "stm32f4xx.h"
#include "stdbool.h"
//*****************************************************************************************************************
bool hex_arr_compare(u8* arr_a, u8* arr_b, u32 cmp_len); // 对比两个十六进制数组从起始位置开始指定长度的数据是否相等
bool xor_checksum_range(u8* data, u32 start, u32 end, u8* result);   // 对数组从指定起始位置到结束位置的数据进行异或校验,并校验合法性
void hex_arr_to_str(u8* hex_arr, u32 arr_len, char* out_str); //  将十六进制字节数组按 %02x 格式存入字符串,如 {0xAB,0x0F} → "ab0f"
u32 str_count_substr(const char *str, const char *sub); // 统计子串 sub 在字符串 str 中出现的次数（不重叠匹配）
bool str_extract_equal(const char *src, const char *start_str, const char *end_str,char *out, u32 out_size)   ; // 截取子串
bool str_intercept(const char *src, const char *start_str, const char *end_str,char *out, u32 out_size) ; // 源字符串中按 start_str~end_str 截取数据，保存到 out
bool str_extract_before_first(const char *src, const char *target, char *out, u32 out_size) ; // 从开始到指定字符之间的字符截取并保存
u32 hex_str_to_bytes_by_len(const char *hex_str, u32 hex_len, u8 *out, u32 out_size);
bool dec_str_to_s8(const char *str, s8 *out);

#endif




