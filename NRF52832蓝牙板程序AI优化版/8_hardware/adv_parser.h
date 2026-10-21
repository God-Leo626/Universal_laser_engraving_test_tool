#ifndef ADV_PARSER_H
#define ADV_PARSER_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"

#define ADV_DATA_MAX_LENGTH 31u
typedef struct {
    uint8_t mac[16];         /* 明文为 16 字节唯一码，XOR 为 6 字节 MAC。 */
    uint8_t mac_length;
    uint8_t sn[7];
    uint8_t sn_length;
} adv_fields_t;

/* 默认模式仅按 RSSI；自定义模式还要求有效特征前缀匹配。 */
bool adv_matches(const uint8_t *data, size_t length, int8_t rssi, const app_config_t *config);
/* 保留两种原始协议：固定偏移明文、校验和 + 码表 XOR。 */
bool adv_decode_fields(const uint8_t *data, size_t length, bool plaintext, adv_fields_t *fields);
#endif
