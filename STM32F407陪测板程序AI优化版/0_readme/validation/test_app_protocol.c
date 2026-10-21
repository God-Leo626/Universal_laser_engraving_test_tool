#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../../6_hardware/app_protocol.h"

static unsigned int frames;
static unsigned int binary_frames;
static unsigned int text_frames;

static void feed(AppPcStream *stream, const uint8_t *data, size_t length)
{
    size_t i, result;
    for (i = 0; i < length; ++i) {
        result = app_pc_feed(stream, data[i]);
        assert(stream->used <= APP_PC_TEXT_MAX);
        if (result) {
            ++frames;
            if (stream->data[0] == 0x7f) {
                assert(app_binary_valid(stream->data, result));
                ++binary_frames;
            } else {
                assert(result >= 2 && stream->data[result - 2] == '\r' && stream->data[result - 1] == '\n');
                assert(stream->data[result] == 0);
                ++text_frames;
            }
        }
    }
}

static void test_stream(void)
{
    AppPcStream stream = {{0}, 0, false};
    uint8_t frame[14] = {0x7f, 7, 0x81, 0, 0, 0xd8, 0};
    const char *commands = "only_len_set=06\r\nprj_set=LASER\r\nstart_scanning\r\n";
    size_t split, length;
    frame[6] = app_xor(frame + 1, 5);
    /* A frame split at every possible DMA boundary, including one byte chunks. */
    for (split = 0; split <= 7; ++split) {
        unsigned int before = frames;
        feed(&stream, frame, split);
        feed(&stream, frame + split, 7 - split);
        assert(frames == before + 1);
    }
    feed(&stream, (const uint8_t *)commands, strlen(commands));
    assert(text_frames == 3);
    /* Six, seven and eight byte feature frames remain separate when concatenated. */
    for (length = 12; length <= 14; ++length) {
        unsigned int before = frames;
        frame[1] = (uint8_t)length; frame[2] = 0x82;
        memset(frame + 5, 0xa5, length - 6);
        frame[length - 1] = app_xor(frame + 1, length - 2);
        feed(&stream, frame, length);
        feed(&stream, frame, length);
        assert(frames == before + 2);
        frame[length - 1] ^= 1;
        feed(&stream, frame, length);
        assert(frames == before + 2);
    }
    /* An oversized line is discarded as a whole, then parsing resumes. */
    for (length = 0; length < 400; ++length) app_pc_feed(&stream, 'x');
    feed(&stream, (const uint8_t *)"\r\nstart_scanning\r\n", 18);
    assert(text_frames == 4);
    assert(binary_frames == 14);
}

static const char *cipher_line = "[RSSI=-22] ---> [CH=37] ---> [tx_pwr=127] ---> [DECRYPT_MAC]=DC5475A87ACC ---> [DECRYPT_SN]=23025100000298\r\n";
static const char *plain_line = "[RSSI=-105] ---> [CH=39] ---> [tx_pwr=127] ---> [DECRYPT_MAC]=00112233445566778899AABBCCDDEEFF ---> [DECRYPT_SN]=23025100000298\r\n";

static void test_reports(void)
{
    AppAdvertisement ad, other;
    struct { unsigned char prefix; uint8_t packet[APP_PACKET_MAX]; unsigned char suffix; } guarded;
    uint8_t ack[7] = {0x7f, 7, 6, 0x80, 0, 0, 0};
    size_t length;
    memset(&guarded, 0xcc, sizeof(guarded));
    assert(app_parse_advertisement(cipher_line, &ad));
    length = app_build_packet(&ad, 6, 6, guarded.packet, sizeof(guarded.packet));
    assert(length == 20 && app_binary_valid(guarded.packet, length));
    assert(guarded.packet[5] == 0xdc && guarded.packet[11] == 0x23 && guarded.packet[18] == 0xea);
    assert(guarded.prefix == 0xcc && guarded.suffix == 0xcc);
    assert(!app_build_packet(&ad, 0, 0, guarded.packet, sizeof(guarded.packet)));
    assert(!app_build_packet(&ad, 131, 131, guarded.packet, sizeof(guarded.packet)));
    assert(!app_build_packet(&ad, 16, 16, guarded.packet, sizeof(guarded.packet)));
    assert(!app_build_packet(&ad, 6, 6, guarded.packet, 19));
    other = ad;
    assert(app_same_product(&ad, &other));
    other.sn[0] = '1';
    assert(!app_same_product(&ad, &other));
    assert(app_parse_advertisement(plain_line, &ad));
    assert(strlen(ad.id) == 32);
    length = app_build_packet(&ad, 16, 0x83, guarded.packet, sizeof(guarded.packet));
    assert(length == 30 && guarded.packet[2] == 0x83 && guarded.packet[28] == (uint8_t)-105);
    assert(guarded.prefix == 0xcc && guarded.suffix == 0xcc);
    ad.sn[0] = 'G';
    assert(!app_build_packet(&ad, 16, 16, guarded.packet, sizeof(guarded.packet)));
    ack[6] = app_xor(ack + 1, 5);
    assert(app_pc_ack_matches(ack, 7, 6));
    assert(!app_pc_ack_matches(ack, 7, 0x83));
    ack[6] ^= 1;
    assert(!app_pc_ack_matches(ack, 7, 6));
}

static void test_invalid_input(void)
{
    AppPcStream stream = {{0}, 0, false};
    AppAdvertisement ad;
    int8_t value;
    uint8_t byte;
    char field[3];
    uint32_t random = 0x12345678U;
    size_t i;
    assert(app_decimal_s8("-128", &value) && value == -128);
    assert(app_decimal_s8("127", &value) && value == 127);
    assert(!app_decimal_s8("128", &value));
    assert(!app_decimal_s8("-129", &value));
    assert(!app_decimal_s8("-", &value));
    assert(!app_decimal_s8("2junk", &value));
    assert(!app_decimal_s8("99999999999999999999", &value));
    assert(!app_hex_bytes("0G", &byte, 1));
    assert(!app_hex_bytes("0", &byte, 1));
    assert(!app_field("only_len_set=060\r\n", "only_len_set=", "\r\n", field, sizeof(field)));
    assert(!app_parse_advertisement("[DECRYPT_MAC]=DC5475A87ACC\r\n", &ad));
    random = 0;
    assert(!app_retry_due(0, &random));
    assert(!app_retry_due(3, &random));
    assert(app_retry_due(4, &random));
    assert(!app_retry_due(4, &random));
    assert(!app_retry_due(7, &random));
    assert(app_retry_due(8, &random));
    assert(!app_retry_due(9, &random));
    random = 0x12345678U;
    for (i = 0; i < 1000000U; ++i) {
        random = random * 1664525U + 1013904223U;
        byte = (uint8_t)(random >> 24);
        feed(&stream, &byte, 1);
    }
}

int main(void)
{
    test_stream();
    test_reports();
    test_invalid_input();
    puts("PASS: stream fragmentation/coalescing, feature lengths, malformed lines, 16-byte ID, packet bounds, ACK, 1000000-byte fuzz");
    return 0;
}
