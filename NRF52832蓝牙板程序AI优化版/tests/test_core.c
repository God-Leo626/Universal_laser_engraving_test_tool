#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include "data_processing.h"
#include "adv_parser.h"
#include "uart_commands.h"
#include "uarte_52832_dma.h"

static unsigned checks;
#define CHECK(expr) do { ++checks; if (!(expr)) { fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
static char output[32768];
static size_t output_length;

void print(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int count = vsnprintf(output + output_length, sizeof(output) - output_length, format, args);
    va_end(args);
    CHECK(count >= 0 && (size_t)count < sizeof(output) - output_length);
    output_length += (size_t)count;
}

bool uart_dma_read(uint8_t *data, size_t capacity, size_t *length, bool *damaged)
{
    (void)data; (void)capacity; (void)length; (void)damaged;
    return false;
}

static void feed(const char *text)
{
    uart_commands_feed((const uint8_t *)text, strlen(text));
}

static void reset(void)
{
    memset(&g_app_config, 0, sizeof(g_app_config));
    memset(feature_raw, 0, sizeof(feature_raw));
    feature_raw_len = 0;
    feature_global_flag = false;
    uart_commands_discard_partial();
    feed("\n");
    output_length = 0;
    output[0] = '\0';
}

static void test_numbers(void)
{
    bool ok;
    char large[100];
    CHECK(num_str_to_num("-105", &ok, 10) == -105 && ok);
    CHECK(num_str_to_num("0", &ok, 10) == 0 && ok);
    CHECK(num_str_to_num("0xff", &ok, 0) == 255 && ok);
    const char *bad[] = {"", "-", "+", "-12x", " -12", "-12 ", "\t2", NULL};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        CHECK(num_str_to_num(bad[i], &ok, 10) == 0 && !ok);
    }
    memset(large, '9', sizeof(large) - 1); large[sizeof(large) - 1] = 0;
    CHECK(num_str_to_num(large, &ok, 10) == 0 && !ok);
    CHECK(num_str_to_num("1", &ok, 1) == 0 && !ok);
    CHECK(num_str_to_num("1", NULL, 10) == 0);
}

static void test_features(void)
{
    reset();
    CHECK(feature_set("0201aF"));
    CHECK(feature_global_flag && feature_raw_len == 3 && feature_raw[2] == 0xaf);
    const char *bad[] = {"", "0", "020x", "GG", "+1", " 1", "0x02", NULL};
    for (size_t i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
        CHECK(!feature_set(bad[i]));
        CHECK(feature_global_flag && feature_raw_len == 3 && feature_raw[2] == 0xaf);
    }
    char long_hex[67];
    memset(long_hex, 'A', 62); long_hex[62] = 0;
    CHECK(feature_set(long_hex) && feature_raw_len == 31 && feature_raw[30] == 0xaa);
    long_hex[62] = 'A'; long_hex[63] = 'A'; long_hex[64] = 0;
    CHECK(!feature_set(long_hex) && feature_raw_len == 31);
    long_hex[64] = 'A'; long_hex[65] = 'A'; long_hex[66] = 0;
    CHECK(!feature_set(long_hex) && feature_raw_len == 31);
    char small[4] = "xxx";
    CHECK(str_extract("a[12]z", "[", "]", small, sizeof(small)) && strcmp(small, "12") == 0);
    CHECK(!str_extract("a[1234]z", "[", "]", small, sizeof(small)) && small[0] == 0);
    CHECK(!str_extract("a[12", "[", "]", small, sizeof(small)));
    CHECK(!str_extract(NULL, "[", "]", small, sizeof(small)));
    CHECK(!str_extract("[]", "[", "]", NULL, 0));
}

static void checksum(uint8_t *packet)
{
    packet[8] = 0;
    for (size_t i = 9; i < 31; ++i) packet[8] = (uint8_t)(packet[8] + packet[i]);
}

static void test_decode(void)
{
    /* Independent known key prefix copied from the original protocol table. */
    static const uint8_t key[21] = {93,104,182,158,107,45,225,8,238,216,83,108,34,74,17,79,221,54,51,124,38};
    static const uint8_t expected[21] = {4,6,0xdc,0x54,0x75,0xa8,0x7a,0xcc,5,7,0x23,2,0x51,0,0,2,0x98,0,0x30,0x38,0x38};
    uint8_t packet[32] = {2,1,5,0x1b,0xff,0xca,0x0a,0xfe};
    struct { uint8_t before; uint8_t bytes[22]; uint8_t after; } guarded;
    memset(&guarded, 0xA5, sizeof(guarded));
    for (size_t i = 0; i < 21; ++i) packet[10 + i] = expected[i] ^ key[i];
    checksum(packet);
    CHECK(decrypt_data(packet, 31, guarded.bytes, 22));
    CHECK(memcmp(guarded.bytes, expected, 21) == 0 && guarded.bytes[21] == 0);
    CHECK(guarded.before == 0xA5 && guarded.after == 0xA5);
    CHECK(!decrypt_data(packet, 31, guarded.bytes, -1));
    CHECK(!decrypt_data(packet, 31, guarded.bytes, 21));
    CHECK(!decrypt_data(NULL, 31, guarded.bytes, 22));
    CHECK(!decrypt_data(packet, 31, NULL, 22));
    for (size_t length = 0; length <= 32; ++length) {
        if (length != 31) CHECK(!decrypt_data(packet, length, guarded.bytes, 22));
    }
    adv_fields_t fields;
    CHECK(adv_decode_fields(packet, 31, false, &fields));
    CHECK(fields.mac_length == 6 && fields.sn_length == 7);
    CHECK(memcmp(fields.mac, expected + 2, 6) == 0 && memcmp(fields.sn, expected + 10, 7) == 0);
    packet[8] ^= 1;
    CHECK(!adv_decode_fields(packet, 31, false, &fields));
    CHECK(fields.mac_length == 0 && fields.sn_length == 0);
    for (unsigned index = 0; index <= 255; ++index) {
        packet[9] = (uint8_t)index; checksum(packet);
        CHECK(decrypt_data(packet, 31, guarded.bytes, 22) == (index <= 234));
        CHECK(guarded.before == 0xA5 && guarded.after == 0xA5);
    }
    for (size_t i = 0; i < 31; ++i) packet[i] = (uint8_t)i;
    CHECK(adv_decode_fields(packet, 31, true, &fields));
    CHECK(fields.mac_length == 16 && fields.sn_length == 7);
    CHECK(fields.mac[0] == 8 && fields.mac[15] == 23 && fields.sn[0] == 24 && fields.sn[6] == 30);
    CHECK(!adv_decode_fields(packet, 30, true, &fields));
    CHECK(!adv_decode_fields(packet, 32, true, &fields));
}

static void test_commands_and_filter(void)
{
    uint8_t matching[] = {2,1,5};
    uint8_t other[] = {2,1,6};
    reset();
    feed("rssi_set=-38\r");
    CHECK(!g_app_config.rssi_configured);
    feed("\nfeature_value_set=020105\r\nmask_raw_data\r\nbyte_order_set\r\ndecoding_setting\r\n");
    CHECK(g_app_config.rssi_configured && g_app_config.rssi_threshold == -38);
    CHECK(g_app_config.show_raw && g_app_config.reverse_address && g_app_config.plaintext);
    CHECK(strstr(output, "[ rssi = -38 , rssi_set_ok ]\r\n") != NULL);
    CHECK(strstr(output, "[ feature_value = 0x020105 ]\r\n") != NULL);
    CHECK(strstr(output, "[ plaintext_set_ok ]\r\n") != NULL);
    feed("rssi_set=-999\r\nrssi_set=\r\nfeature_value_set=GG\r\n");
    CHECK(g_app_config.rssi_configured && g_app_config.rssi_threshold == -38);
    CHECK(feature_raw_len == 3 && feature_raw[2] == 5);
    feed("start_scanning\r\nrssi_set=-1\r\ndecoding_setting\r\n");
    CHECK(g_app_config.scanning_started && !g_app_config.default_mode);
    CHECK(g_app_config.rssi_threshold == -38 && g_app_config.plaintext);
    CHECK(adv_matches(matching, 3, -38, &g_app_config));
    CHECK(!adv_matches(matching, 3, -39, &g_app_config));
    CHECK(!adv_matches(other, 3, -37, &g_app_config));
    CHECK(!adv_matches(matching, 2, -37, &g_app_config));
    reset();
    feed("xxstart_scanning\r\nstart_scanningoops\r\nuse_default_settings\n");
    CHECK(!g_app_config.scanning_started);
    feed("feature_value_set=020105\r\nuse_default_settings\r\n");
    CHECK(g_app_config.default_mode && g_app_config.show_raw && g_app_config.rssi_threshold == -60);
    CHECK(adv_matches(matching, 3, -60, &g_app_config));
    CHECK(adv_matches(other, 3, -60, &g_app_config)); /* default mode must not change after a feature match */
    CHECK(g_app_config.default_mode);
    CHECK(!adv_matches(other, 3, -61, &g_app_config));
    reset();
    feed("rssi_set=-105\r\ndecoding_setting\r\ndecoding_setting\r\nuse_default_settings\r\n");
    CHECK(g_app_config.rssi_threshold == -105 && !g_app_config.plaintext);
    CHECK(strstr(output, "[ cipnertext_set_ok ]\r\n") != NULL);
    reset();
    uint8_t zeros[] = {'s','t','a','r','t',0,'_','s','c','a','n','n','i','n','g','\r','\n'};
    uart_commands_feed(zeros, sizeof(zeros));
    CHECK(!g_app_config.scanning_started);
    char huge[400]; memset(huge, 'x', sizeof(huge));
    uart_commands_feed((uint8_t *)huge, sizeof(huge));
    feed("start_scanning\r\n");
    CHECK(!g_app_config.scanning_started);
    feed("use_default_settings\r\n");
    CHECK(g_app_config.scanning_started);
}

int main(void)
{
    test_numbers(); test_features(); test_decode(); test_commands_and_filter();
    printf("Core regression PASS: %u checks\n", checks);
    return 0;
}
