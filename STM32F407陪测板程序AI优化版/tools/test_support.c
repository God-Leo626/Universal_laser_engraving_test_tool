/* Host-only flash bus simulation. No STM32 registers or actual hardware are accessed. */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int32_t s32;

#include "data_processing_under_test.c"

#define W25X_WriteEnable 0x06
#define W25X_WriteDisable 0x04
#define W25X_ReadStatusReg1 0x05
#define W25X_ReadData 0x03
#define W25X_PageProgram 0x02
#define W25X_SectorErase 0x20
#define W25X_ChipErase 0xc7
#define W25X_PowerDown 0xb9
#define W25X_ReleasePowerDown 0xab
#define W25X_DeviceID 0xab
#define W25X_JedecDeviceID 0x9f
#define DUMMY_DATA 0xff

static int cs[2] = {1, 1}, previous_cs[2] = {1, 1};
static u8 storage[2][4 * 1024 * 1024];
static u8 SPI1_FLASH_BUF[4096];
static unsigned int delays, transactions, programs, erases;
static int fail_transfer_after = -1;
static bool byte_ok = true, force_busy = false, deny_write_enable = false;
typedef struct { u8 cmd; unsigned int count, address, offset, busy; bool wel; } Chip;
static Chip chip[2];

static void sync_cs(void)
{
    int i;
    assert(cs[0] != 0 || cs[1] != 0);
    for (i = 0; i < 2; ++i)
    {
        Chip *c = &chip[i];
        if (previous_cs[i] == 1 && cs[i] == 0)
        {
            c->cmd = 0; c->count = 0; c->address = 0; c->offset = 0;
            ++transactions;
        }
        if (previous_cs[i] == 0 && cs[i] == 1)
        {
            if (c->cmd == W25X_PageProgram && c->wel)
            { c->wel = false; c->busy = 2; ++programs; }
            if (c->cmd == W25X_SectorErase && c->wel)
            {
                assert(c->address < sizeof(storage[i]));
                memset(storage[i] + (c->address & ~4095U), 0xff, 4096);
                c->wel = false; c->busy = 2; ++erases;
            }
            if (c->cmd == W25X_ChipErase && c->wel)
            { memset(storage[i], 0xff, sizeof(storage[i])); c->wel = false; c->busy = 2; }
        }
        previous_cs[i] = cs[i];
    }
}

static int *mock_cs(int index) { sync_cs(); return &cs[index]; }
#define W25Q32_CS_PA4 (*mock_cs(0))
#define W25Q32_CS_PC1 (*mock_cs(1))

static u8 spi1_read_write_byte(u8 value)
{
    Chip *c;
    int index;
    u8 result = 0xff;
    sync_cs();
    byte_ok = fail_transfer_after != 0;
    if (fail_transfer_after > 0) --fail_transfer_after;
    if (!byte_ok) return 0xff;
    index = cs[0] == 0 ? 0 : 1;
    assert(cs[index] == 0);
    c = &chip[index];
    if (c->count++ == 0)
    {
        c->cmd = value;
        if (value == W25X_WriteEnable && !deny_write_enable) c->wel = true;
        if (value == W25X_WriteDisable) c->wel = false;
        return 0xff;
    }
    if (c->cmd == W25X_ReadStatusReg1)
    {
        result = (u8)((force_busy || c->busy != 0 ? 1 : 0) | (c->wel ? 2 : 0));
        if (c->busy != 0) --c->busy;
    }
    else if (c->cmd == W25X_ReadData || c->cmd == W25X_PageProgram || c->cmd == W25X_SectorErase)
    {
        if (c->count <= 4) c->address = (c->address << 8) | value;
        else
        {
            unsigned int address = c->address + c->offset++;
            assert(address < sizeof(storage[index]));
            if (c->cmd == W25X_ReadData) result = storage[index][address];
            else if (c->cmd == W25X_PageProgram)
            {
                assert(c->wel);
                assert((c->address / 256) == (address / 256));
                storage[index][address] &= value;
            }
        }
    }
    else if (c->cmd == W25X_JedecDeviceID)
    {
        static const u8 id[] = {0xef, 0x40, 0x16};
        result = id[c->count - 2];
    }
    else if (c->cmd == W25X_DeviceID && c->count == 5) result = 0x15;
    return result;
}

static bool spi1_last_transfer_ok(void) { return byte_ok; }
static void systick_delay_ms(u32 value) { delays += value; }
static void systick_delay_us(u32 value) { (void)value; }

#include "w25q32_under_test.c"

static void test_data(void)
{
    struct { char text[4]; char guard; } out = {{'k', 'e', 'e', 'p'}, 'G'};
    u8 bytes[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    char text[64];
    s8 signed_value;
    u8 checksum;
    assert(hex_str_to_bytes_by_len("123456", 6, bytes, 2) == 0);
    assert(bytes[0] == 0xaa && bytes[1] == 0xbb);
    assert(hex_str_to_bytes_by_len("12z4", 4, bytes, 4) == 0 && bytes[0] == 0xaa);
    assert(hex_str_to_bytes_by_len(NULL, 4, bytes, 4) == 0);
    assert(hex_str_to_bytes_by_len("aB01", 4, bytes, 4) == 2 && bytes[0] == 0xab && bytes[1] == 1);
    hex_arr_to_str(bytes, 2, text); assert(strcmp(text, "ab01") == 0);
    hex_arr_to_str(NULL, 0, text); assert(text[0] == 0);
    assert(xor_checksum_range(bytes, 0, 1, &checksum) && checksum == 0xaa);
    assert(!xor_checksum_range(bytes, 2, 1, &checksum));
    assert(dec_str_to_s8("-128", &signed_value) && signed_value == -128);
    assert(dec_str_to_s8("+127", &signed_value) && signed_value == 127);
    assert(!dec_str_to_s8("128", &signed_value) && !dec_str_to_s8("-129", &signed_value));
    assert(!dec_str_to_s8("+", &signed_value) && !dec_str_to_s8(NULL, &signed_value));
    assert(str_count_substr("aaaaa", "aa") == 2 && str_count_substr(NULL, "a") == 0);
    assert(str_intercept("key=AB\r\n", "key=", "\r\n", out.text, 3));
    assert(strcmp(out.text, "AB") == 0 && out.guard == 'G');
    memcpy(out.text, "keep", 4);
    assert(!str_intercept("key=ABCD\r\n", "key=", "\r\n", out.text, 4));
    assert(memcmp(out.text, "keep", 4) == 0 && out.guard == 'G');
    assert(!str_intercept("x", "", "z", out.text, 0));
    assert(!str_extract_before_first("abcdef\r\n", "\r\n", out.text, 4));
    assert(memcmp(out.text, "keep", 4) == 0);
    assert(!str_extract_before_first("abc", "\r\n", out.text, 4));
    assert(memcmp(out.text, "keep", 4) == 0);
    assert(str_extract_before_first("abc\r\n", "\r\n", out.text, 4));
    assert(strcmp(out.text, "abc") == 0 && out.guard == 'G');
    assert(str_extract_equal("[mac=ABC];[mac=ABC];", "[mac=", "];", text, sizeof(text)));
    assert(strcmp(text, "ABC") == 0);
    assert(!str_extract_equal("[mac=ABC];[mac=ABD];", "[mac=", "];", text, sizeof(text)));
    assert(!str_extract_equal("[mac=ABC];[mac=ABC];[mac=", "[mac=", "];", text, sizeof(text)));
    assert(!str_extract_equal("[mac=ABC];[mac=ABC];", "[mac=", "];", out.text, 3));
    puts("PASS data: exact-capacity, null, zero, invalid hex, no partial writes, signed bounds, equal fields");
}

static void test_flash(void)
{
    u8 source[800], actual[800], sector_before[4096];
    unsigned int i, previous_count;
    memset(storage, 0xff, sizeof(storage));
    for (i = 0; i < sizeof(source); ++i) source[i] = (u8)(i * 37 + 5);
    assert(w25q32_flash_read_jedec_id(2) == 0xef4016 && w25q32_last_operation_ok());
    assert(w25q32_flash_read_device_id(1) == 0x15);
    /* Unaligned write spans four pages; command model asserts no page wrapping. */
    w25q32_flash_autopage_write_no_erase(source, 251, sizeof(source), 1);
    assert(w25q32_last_operation_ok());
    w25q32_flash_read(actual, 251, sizeof(actual), 1);
    assert(w25q32_last_operation_ok() && memcmp(source, actual, sizeof(source)) == 0);
    assert(storage[0][250] == 0xff && storage[0][1051] == 0xff && storage[1][251] == 0xff);
    /* Two prepopulated sectors: preserve bytes preceding/following replacement. */
    for (i = 0; i < 8192; ++i) storage[1][i] = (u8)(i * 7 + 11);
    memcpy(sector_before, storage[1], sizeof(sector_before));
    w25q32_flash_autosector_write_with_erase(source, 4000, sizeof(source), 2);
    assert(w25q32_last_operation_ok() && erases == 2);
    assert(memcmp(storage[1], sector_before, 4000) == 0);
    assert(memcmp(storage[1] + 4000, source, sizeof(source)) == 0);
    for (i = 4800; i < 8192; ++i) assert(storage[1][i] == (u8)(i * 7 + 11));
    previous_count = transactions;
    w25q32_flash_page256_write_no_erase(source, 255, 2, 2); assert(!w25q32_last_operation_ok());
    w25q32_flash_read(actual, FLASH_SIZE - 1, 2, 2); assert(!w25q32_last_operation_ok());
    w25q32_flash_sector4k_erase(1024, 2); assert(!w25q32_last_operation_ok());
    w25q32_flash_autosector_write_with_erase(SPI1_FLASH_BUF, 0, 1, 2); assert(!w25q32_last_operation_ok());
    w25q32_flash_read(actual, 0, 1, 3); assert(!w25q32_last_operation_ok());
    assert(transactions == previous_count);
    w25q32_flash_autopage_write_no_erase(source, 0, 0, 2); assert(w25q32_last_operation_ok());
    assert(transactions == previous_count);
    /* Erased destination does not require an erase. */
    previous_count = erases;
    w25q32_flash_autosector_write_with_erase(source, 9000, 500, 2);
    assert(w25q32_last_operation_ok() && erases == previous_count);
    deny_write_enable = true;
    w25q32_flash_autopage_write_no_erase(source, 10000, 1, 2);
    assert(!w25q32_last_operation_ok() && storage[1][10000] == 0xff);
    deny_write_enable = false;
    force_busy = true; delays = 0;
    w25q32_flash_read(actual, 0, sizeof(actual), 2);
    assert(!w25q32_last_operation_ok() && delays == READY_TIMEOUT_MS);
    for (i = 0; i < sizeof(actual); ++i) assert(actual[i] == 0xff);
    previous_count = erases;
    w25q32_flash_autosector_write_with_erase(source, 0, 1, 2);
    assert(!w25q32_last_operation_ok() && erases == previous_count);
    force_busy = false; fail_transfer_after = 1;
    w25q32_flash_read(actual, 0, 10, 2);
    assert(!w25q32_last_operation_ok());
    for (i = 0; i < 10; ++i) assert(actual[i] == 0xff);
    sync_cs(); assert(cs[0] == 1 && cs[1] == 1);
    puts("PASS flash: select isolation, page/sector boundaries, unchanged neighboring bytes, range rejection, zero length, WEL failure, busy timeout, SPI fault");
}

int main(void)
{
    test_data();
    test_flash();
    puts("Host simulation only: verify SPI timing, erase/program limits and power-loss behavior on hardware.");
    return 0;
}
