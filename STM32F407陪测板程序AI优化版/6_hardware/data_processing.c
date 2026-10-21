/* 通用数据处理：所有字符串输入必须以 '\0' 结束；失败时不改写输出。
 * 函数不分配大数组，协议/业务层负责检查原始接收长度与字段语义。 */
#include "data_processing.h"
#include <stddef.h>
#include <string.h>

bool hex_arr_compare(u8 *arr_a, u8 *arr_b, u32 cmp_len)
{
    return arr_a != NULL && arr_b != NULL && cmp_len != 0 &&
           memcmp(arr_a, arr_b, cmp_len) == 0;
}

/* end 为包含端点；本接口没有数组容量，调用者必须保证 end 未越界。 */
bool xor_checksum_range(u8 *data, u32 start, u32 end, u8 *result)
{
    u8 checksum = 0;
    u32 i;
    if (data == NULL || result == NULL || start > end) return false;
    for (i = start; ; ++i)
    {
        checksum ^= data[i];
        if (i == end) break; /* 避免 end == UINT32_MAX 时 i 回绕导致死循环。 */
    }
    *result = checksum;
    return true;
}

/* 小写输出；调用者保证 out_str 至少有 arr_len * 2 + 1 字节。 */
void hex_arr_to_str(u8 *hex_arr, u32 arr_len, char *out_str)
{
    static const char digits[] = "0123456789abcdef";
    u32 i;
    if (out_str == NULL || (arr_len != 0 && hex_arr == NULL) ||
        arr_len > (0xffffffffUL - 1) / 2) return;
    for (i = 0; i < arr_len; ++i)
    {
        out_str[i * 2] = digits[hex_arr[i] >> 4];
        out_str[i * 2 + 1] = digits[hex_arr[i] & 0x0f];
    }
    out_str[arr_len * 2] = '\0';
}

static int hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

/* 完整转换才返回字节数。容量不足或任意非法字符时返回0，输出保持原样。 */
u32 hex_str_to_bytes_by_len(const char *hex_str, u32 hex_len, u8 *out, u32 out_size)
{
    u32 i;
    if (hex_str == NULL || out == NULL || hex_len == 0 ||
        (hex_len & 1) != 0 || hex_len / 2 > out_size) return 0;
    for (i = 0; i < hex_len; ++i)
        if (hex_digit(hex_str[i]) < 0) return 0;
    for (i = 0; i < hex_len; i += 2)
        out[i / 2] = (u8)((hex_digit(hex_str[i]) << 4) | hex_digit(hex_str[i + 1]));
    return hex_len / 2;
}

bool dec_str_to_s8(const char *str, s8 *out)
{
    s32 value = 0;
    s32 sign = 1;
    if (str == NULL || out == NULL) return false;
    if (*str == '-') { sign = -1; ++str; }
    else if (*str == '+') ++str;
    if (*str == '\0') return false;
    while (*str != '\0')
    {
        if (*str < '0' || *str > '9') return false;
        value = value * 10 + (*str++ - '0');
        if (value > 128) return false;
    }
    value *= sign;
    if (value < -128 || value > 127) return false;
    *out = (s8)value;
    return true;
}

u32 str_count_substr(const char *str, const char *sub)
{
    u32 count = 0;
    size_t len;
    if (str == NULL || sub == NULL || *sub == '\0') return 0;
    len = strlen(sub);
    while ((str = strstr(str, sub)) != NULL) { ++count; str += len; }
    return count;
}

/* 至少两个完整、非空字段且逐字节相同才成功；不截断，不使用2KB栈缓冲。 */
bool str_extract_equal(const char *src, const char *start_str, const char *end_str,
                       char *out, u32 out_size)
{
    const char *first = NULL, *begin, *end;
    size_t first_len = 0, len, start_len, end_len;
    u32 count = 0;
    if (src == NULL || start_str == NULL || end_str == NULL || out == NULL ||
        out_size == 0 || *start_str == '\0' || *end_str == '\0') return false;
    start_len = strlen(start_str);
    end_len = strlen(end_str);
    while ((begin = strstr(src, start_str)) != NULL)
    {
        begin += start_len;
        end = strstr(begin, end_str);
        if (end == NULL) return false;
        len = (size_t)(end - begin);
        if (len == 0 || len >= out_size) return false;
        if (first == NULL) { first = begin; first_len = len; }
        else if (len != first_len || memcmp(first, begin, len) != 0) return false;
        ++count;
        src = end + end_len;
    }
    if (count < 2) return false;
    memmove(out, first, first_len);
    out[first_len] = '\0';
    return true;
}

bool str_intercept(const char *src, const char *start_str, const char *end_str,
                   char *out, u32 out_size)
{
    const char *begin, *end;
    size_t len;
    if (src == NULL || start_str == NULL || end_str == NULL || out == NULL ||
        out_size == 0 || *start_str == '\0' || *end_str == '\0') return false;
    begin = strstr(src, start_str);
    if (begin == NULL) return false;
    begin += strlen(start_str);
    end = strstr(begin, end_str);
    if (end == NULL) return false;
    len = (size_t)(end - begin);
    if (len == 0 || len >= out_size) return false;
    memmove(out, begin, len);
    out[len] = '\0';
    return true;
}

/* 从开头取到首个标记之前。标记可紧邻开头（成功返回空串）。 */
bool str_extract_before_first(const char *src, const char *target, char *out, u32 out_size)
{
    const char *end;
    size_t len;
    if (src == NULL || target == NULL || out == NULL || out_size == 0 || *target == '\0')
        return false;
    end = strstr(src, target);
    if (end == NULL) return false;
    len = (size_t)(end - src);
    if (len >= out_size) return false;
    memmove(out, src, len);
    out[len] = '\0';
    return true;
}
