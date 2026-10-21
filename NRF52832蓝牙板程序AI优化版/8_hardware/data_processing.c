/** 命令参数转换与固定广播协议的 XOR 解码；不依赖硬件或串口。 */
#include "data_processing.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

uint8_t feature_raw[FEATURE_RAW_CAPACITY];
uint8_t feature_raw_len;
bool feature_global_flag;

/* 保留原协议码表。XOR 是设备协议编码，不提供密码学安全性。 */
static const uint8_t encrypt_key[255] = 
{
      93,104,182,158,107,45,225,8,238,216,83,108,34,74,17,79,221,
      54,51,124,38,14,155,23,156,184,151,63,131,168,140,13,12,185,
      58,235,98,237,5,165,2,204,19,250,175,53,68,9,6,39,248,181,90,
      92,48,207,127,77,147,219,242,199,16,133,67,186,18,66,239,57,
      201,231,146,123,97,95,191,3,71,214,234,215,223,217,169,41,222,
      85,157,117,7,89,240,161,129,72,137,73,149,167,174,121,144,84,
      70,195,152,10,62,198,243,145,91,236,244,120,100,94,209,212,25,
      60,218,59,229,119,138,245,75,226,103,220,31,116,230,26,29,210,
      109,135,80,56,196,24,46,50,11,36,81,194,110,162,102,200,211,172,
      160,69,118,33,113,134,86,197,164,205,142,65,170,44,154,37,136,47,
      64,15,183,254,150,190,82,76,1,246,178,30,192,241,40,233,171,251,
      159,255,173,111,224,177,87,179,122,208,228,78,128,43,227,99,206,
      28,96,247,21,180,42,125,114,20,189,148,193,27,88,32,61,132,188,
      176,166,213,141,22,106,35,112,4,249,202,115,252,52,203,232,49,
      126,105,101,187,143,163,139,153,55,253,130
};

static int hex_digit(char ch)
{
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
    if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
    return -1;
}

bool feature_set(const char *hex_str)
{
    uint8_t parsed[FEATURE_RAW_CAPACITY];
    size_t length;
    if (hex_str == NULL) return false;
    length = strlen(hex_str);
    if (length == 0 || (length & 1u) != 0 || length > sizeof(parsed) * 2u) {
        return false;
    }
    for (size_t i = 0; i < length; i += 2) {
        int high = hex_digit(hex_str[i]);
        int low = hex_digit(hex_str[i + 1]);
        if (high < 0 || low < 0) return false;
        parsed[i / 2] = (uint8_t)((high << 4) | low);
    }
    /* 整个参数验证通过后一次提交，错误命令不会破坏先前有效配置。 */
    memset(feature_raw, 0, sizeof(feature_raw));
    memcpy(feature_raw, parsed, length / 2);
    feature_raw_len = (uint8_t)(length / 2);
    feature_global_flag = true;
    return true;
}

long num_str_to_num(const char *text, bool *ok, uint8_t base)
{
    char *end;
    long value;
    if (ok == NULL) return 0;
    *ok = false;
    if (text == NULL || *text == '\0' || (base != 0 && (base < 2 || base > 36))) return 0;
    /* 参数中不接受空白、部分数字或只有正负号的字符串。 */
    for (const char *p = text; *p != '\0'; ++p) {
        if (*p == ' ' || (*p >= '\t' && *p <= '\r')) return 0;
    }
    errno = 0;
    value = strtol(text, &end, base);
    if (errno == ERANGE || end == text || *end != '\0') return 0;
    *ok = true;
    return value;
}

bool str_extract(const char *src, const char *start, const char *end,
                 char *dst, size_t dst_size)
{
    const char *first;
    const char *last;
    size_t length;
    if (dst == NULL || dst_size == 0) return false;
    dst[0] = '\0';
    if (src == NULL || start == NULL || end == NULL || *start == '\0' || *end == '\0') return false;
    first = strstr(src, start);
    if (first == NULL) return false;
    first += strlen(start);
    last = strstr(first, end);
    if (last == NULL) return false;
    length = (size_t)(last - first);
    if (length >= dst_size) return false;
    memcpy(dst, first, length);
    dst[length] = '\0';
    return true;
}

bool decrypt_data(const uint8_t *data, size_t length, uint8_t *output, int output_size)
{
    uint8_t checksum = 0;
    size_t key_index;
    /* 必须先验证指针与有符号容量，不能先用负数调用 memset。 */
    if (data == NULL || output == NULL || length != ENCRYPTED_PACKET_SIZE ||
        output_size < (int)DECRYPTED_BUFFER_SIZE) return false;
    key_index = data[9];
    if (key_index > sizeof(encrypt_key) - DECRYPTED_DATA_SIZE) return false;
    for (size_t i = 9; i < ENCRYPTED_PACKET_SIZE; ++i) checksum = (uint8_t)(checksum + data[i]);
    if (checksum != data[8]) return false;
    for (size_t i = 0; i < DECRYPTED_DATA_SIZE; ++i) output[i] = data[i + 10] ^ encrypt_key[key_index + i];
    output[DECRYPTED_DATA_SIZE] = 0;
    return true;
}
