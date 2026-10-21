#ifndef APP_PROTOCOL_H
#define APP_PROTOCOL_H

/* Pure protocol helpers: no STM32/RTOS dependencies; exercised by host tests. */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define APP_PC_TEXT_MAX 64U
#define APP_ID_MAX_BYTES 16U
#define APP_PACKET_MAX (14U + APP_ID_MAX_BYTES)

typedef struct {
    uint8_t data[APP_PC_TEXT_MAX + 1U];
    size_t used;
    bool discard_line;
} AppPcStream;

typedef struct {
    char id[2U * APP_ID_MAX_BYTES + 1U];
    char sn[15];
    char channel[4];
    char rssi[5];
} AppAdvertisement;

static uint8_t app_xor(const uint8_t *data, size_t count)
{
    uint8_t value = 0;
    size_t i;
    for (i = 0; i < count; ++i) value ^= data[i];
    return value;
}

static bool app_binary_valid(const uint8_t *frame, size_t len)
{
    return frame != NULL && len >= 7U && frame[0] == 0x7f &&
           frame[1] == len && app_xor(frame + 1, len - 2U) == frame[len - 1U];
}

/* Returns one complete binary frame or CRLF command in stream->data.
 * The caller consumes it immediately; partial input survives DMA boundaries. */
static size_t app_pc_feed(AppPcStream *stream, uint8_t byte)
{
    size_t len, i;
    if (stream->discard_line) {
        if (byte == '\n') stream->discard_line = false;
        return 0;
    }
    /* A binary header is an unambiguous resynchronization point in ASCII input. */
    if (byte == 0x7f && stream->used != 0U && stream->data[0] != 0x7f) stream->used = 0;
    if (stream->used == 0U && byte != 0x7f && (byte < 32U || byte > 126U)) return 0;
    if (stream->used >= APP_PC_TEXT_MAX) {
        stream->used = 0;
        stream->discard_line = byte != '\n';
        return 0;
    }
    stream->data[stream->used++] = byte;
    stream->data[stream->used] = 0;
    if (stream->data[0] == 0x7f) {
        if (stream->used < 2U) return 0;
        len = stream->data[1];
        if (len != 7U && len != 12U && len != 13U && len != 14U) {
            stream->used = byte == 0x7f ? 1U : 0U;
            return 0;
        }
        if (stream->used < len) return 0;
        if (app_binary_valid(stream->data, len)) {
            stream->used = 0;
            return len;
        }
        /* A damaged frame can contain the beginning of the next frame. */
        for (i = 1U; i < stream->used; ++i) if (stream->data[i] == 0x7f) break;
        stream->used -= i;
        memmove(stream->data, stream->data + i, stream->used);
        return 0;
    }
    if (stream->used >= 2U && byte == '\n' && stream->data[stream->used - 2U] == '\r') {
        len = stream->used;
        stream->used = 0;
        return len;
    }
    if (byte == '\n' || byte == 0U) stream->used = 0;
    return 0;
}

static int app_hex_digit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Require the entire string, including its length, to match the requested data. */
static bool app_hex_bytes(const char *text, uint8_t *out, size_t bytes)
{
    size_t i;
    int high, low;
    if (text == NULL || out == NULL || strlen(text) != 2U * bytes) return false;
    for (i = 0; i < bytes; ++i) {
        high = app_hex_digit(text[2U * i]);
        low = app_hex_digit(text[2U * i + 1U]);
        if (high < 0 || low < 0) return false;
        out[i] = (uint8_t)(high * 16 + low);
    }
    return true;
}

static bool app_decimal_s8(const char *text, int8_t *out)
{
    unsigned int value = 0;
    bool negative = false;
    if (text == NULL || out == NULL) return false;
    if (*text == '-' || *text == '+') { negative = *text == '-'; ++text; }
    if (*text == 0) return false;
    while (*text) {
        if (*text < '0' || *text > '9') return false;
        value = value * 10U + (unsigned int)(*text++ - '0');
        if (value > (negative ? 128U : 127U)) return false;
    }
    *out = (int8_t)(negative ? -(int)value : (int)value);
    return true;
}

static bool app_field(const char *source, const char *start, const char *end,
                      char *output, size_t capacity)
{
    const char *first = strstr(source, start), *last;
    size_t len;
    if (capacity == 0U) return false;
    output[0] = 0;
    if (first == NULL) return false;
    first += strlen(start);
    last = strstr(first, end);
    if (last == NULL) return false;
    len = (size_t)(last - first);
    if (len == 0U || len >= capacity) return false;
    memcpy(output, first, len);
    output[len] = 0;
    return true;
}

static bool app_parse_advertisement(const char *line, AppAdvertisement *ad)
{
    uint8_t scratch[APP_ID_MAX_BYTES];
    int8_t rssi;
    size_t id_len;
    memset(ad, 0, sizeof(*ad));
    if (!app_field(line, "---> [DECRYPT_MAC]=", " ---> [", ad->id, sizeof(ad->id)) ||
        !app_field(line, "---> [DECRYPT_SN]=", "\r\n", ad->sn, sizeof(ad->sn)) ||
        !app_field(line, "---> [CH=", "] ---> [tx_pwr", ad->channel, sizeof(ad->channel)) ||
        !app_field(line, "[RSSI=", "] ---> [CH", ad->rssi, sizeof(ad->rssi))) return false;
    id_len = strlen(ad->id);
    return id_len > 0U && id_len % 2U == 0U &&
           app_hex_bytes(ad->id, scratch, id_len / 2U) &&
           app_hex_bytes(ad->sn, scratch, 7U) && app_decimal_s8(ad->rssi, &rssi);
}

static bool app_same_product(const AppAdvertisement *left, const AppAdvertisement *right)
{
    return strcmp(left->id, right->id) == 0 && strcmp(left->sn, right->sn) == 0;
}

static size_t app_build_packet(const AppAdvertisement *ad, uint8_t id_bytes,
                               uint8_t command, uint8_t *packet, size_t capacity)
{
    int8_t rssi;
    size_t length = 14U + id_bytes;
    if (id_bytes == 0U || id_bytes > APP_ID_MAX_BYTES || capacity < length) return 0;
    memset(packet, 0, length);
    if (!app_hex_bytes(ad->id, packet + 5, id_bytes) ||
        !app_hex_bytes(ad->sn, packet + 5U + id_bytes, 7U) ||
        !app_decimal_s8(ad->rssi, &rssi)) return 0;
    packet[0] = 0x7f;
    packet[1] = (uint8_t)length;
    packet[2] = command;
    packet[length - 2U] = (uint8_t)rssi;
    packet[length - 1U] = app_xor(packet + 1, length - 2U);
    return length;
}

static bool app_pc_ack_matches(const uint8_t *frame, size_t length, uint8_t command)
{
    return length == 7U && app_binary_valid(frame, length) && frame[2] == command &&
           frame[3] == 0x80 && frame[4] == 0U && frame[5] == 0U;
}

/* Timer ticks are 500 ms; a retry is due once at 2 s, then once at 4 s. */
static bool app_retry_due(uint32_t ticks, uint32_t *last_slot)
{
    uint32_t slot = ticks / 4U;
    if (slot <= *last_slot) return false;
    *last_slot = slot;
    return true;
}

#endif
