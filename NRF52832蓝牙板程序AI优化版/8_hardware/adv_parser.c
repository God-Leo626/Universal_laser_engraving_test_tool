#include "adv_parser.h"
#include "data_processing.h"
#include <string.h>

bool adv_matches(const uint8_t *data, size_t length, int8_t rssi, const app_config_t *config)
{
    if (data == NULL || config == NULL || length > ADV_DATA_MAX_LENGTH ||
        !config->rssi_configured || rssi < config->rssi_threshold) return false;
    if (config->default_mode) return true;
    return feature_global_flag && feature_raw_len > 0 && feature_raw_len <= sizeof(feature_raw) &&
           feature_raw_len <= length && memcmp(data, feature_raw, feature_raw_len) == 0;
}

bool adv_decode_fields(const uint8_t *data, size_t length, bool plaintext, adv_fields_t *fields)
{
    uint8_t decoded[DECRYPTED_BUFFER_SIZE];
    if (fields == NULL) return false;
    memset(fields, 0, sizeof(*fields));
    if (data == NULL || length != ADV_DATA_MAX_LENGTH) return false;
    if (plaintext) {
        memcpy(fields->mac, data + 8, 16);
        fields->mac_length = 16;
        memcpy(fields->sn, data + 24, 7);
        fields->sn_length = 7;
    } else {
        if (!decrypt_data(data, length, decoded, sizeof(decoded))) return false;
        /* 原协议是固定位置标签，不将长度字节直接用作任意拷贝长度。 */
        if (decoded[0] == 0x04 && decoded[1] == 6) {
            memcpy(fields->mac, decoded + 2, 6);
            fields->mac_length = 6;
        }
        if (decoded[8] == 0x05 && decoded[9] == 7) {
            memcpy(fields->sn, decoded + 10, 7);
            fields->sn_length = 7;
        }
    }
    return fields->mac_length != 0 || fields->sn_length != 0;
}
