/* Actual production Flash/data code with a transaction-level SPI NOR model.
 * This tests storage algorithm and faults, not electrical timing. */
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "stm32f4xx.h"
#define _IO_BIT_H_
#define __SYSTICK_DElAY_FREERTOS_H__
static unsigned int *model_pin(unsigned int chip);
#define PAout(n) (*model_pin(0))
#define PCout(n) (*model_pin(1))
void systick_delay_ms(u32 ms);
void systick_delay_us(u32 us);
#include "../../../6_hardware/w25q32.c"
#include "../../../6_hardware/data_processing.c"

uint8_t SPI1_FLASH_BUF[4096];
static u8 memory[2][4U * 1024U * 1024U];
static unsigned int cs[2] = {1, 1}, previous_cs[2] = {1, 1};
static unsigned int phase, command_code, selected, erases, transactions;
static u32 address, page_start, programmed;
static bool wel[2], detached, spi_failure, locked;
static u32 delay_count;

static void model_sync(void)
{
    unsigned int chip;
    for (chip = 0; chip < 2; ++chip) {
        if (previous_cs[chip] == 0 && cs[chip] == 1) {
            if (command_code == W25X_SectorErase && phase >= 4 && wel[chip]) {
                memset(memory[chip] + (address & ~4095U), 0xff, 4096);
                ++erases; wel[chip] = false;
            }
            if (command_code == W25X_PageProgram && phase >= 4) wel[chip] = false;
        }
        if (previous_cs[chip] == 1 && cs[chip] == 0) {
            selected = chip; command_code = 0; phase = 0; address = 0; programmed = 0;
            ++transactions;
        }
        previous_cs[chip] = cs[chip];
    }
}

static unsigned int *model_pin(unsigned int chip)
{
    model_sync();
    return &cs[chip];
}

u8 spi1_read_write_byte(u8 value)
{
    model_sync();
    assert(cs[selected] == 0 && cs[1U - selected] == 1);
    if (detached || spi_failure) return 0xff;
    if (phase++ == 0) {
        command_code = value;
        if (value == W25X_WriteEnable && !locked) wel[selected] = true;
        if (value == W25X_WriteDisable) wel[selected] = false;
        return 0xff;
    }
    if (command_code == W25X_ReadStatusReg1) return wel[selected] ? 2 : 0;
    if (command_code == W25X_ReadData || command_code == W25X_PageProgram || command_code == W25X_SectorErase) {
        if (phase <= 4) {
            address = (address << 8) | value;
            if (phase == 4) page_start = address & ~255U;
            return 0xff;
        }
        if (command_code == W25X_ReadData) return memory[selected][address++];
        if (command_code == W25X_PageProgram) {
            assert(wel[selected]);
            memory[selected][page_start | ((address + programmed++) & 255U)] &= value;
        }
    }
    return 0xff;
}

bool spi1_last_transfer_ok(void) { return !spi_failure; }
void systick_delay_ms(u32 ms) { delay_count += ms; }
void systick_delay_us(u32 us) { (void)us; }

static void test_flash(void)
{
    u8 payload[700], expected[8192], out[700];
    unsigned int i, before;
    memset(memory, 0xff, sizeof(memory));
    for (i = 0; i < sizeof(payload); ++i) payload[i] = (u8)(i * 17U + 1U);
    /* Non-erasing multi-page write beginning near a page end. */
    w25q32_flash_autopage_write_no_erase(payload, 250, sizeof(payload), 2);
    model_sync();
    assert(w25q32_last_operation_ok());
    assert(memcmp(memory[1] + 250, payload, sizeof(payload)) == 0);
    assert(memory[1][249] == 0xff && memory[1][950] == 0xff && erases == 0);
    /* Erase/write across two sectors must preserve every unrelated byte. */
    for (i = 0; i < sizeof(expected); ++i) expected[i] = memory[1][i] = (u8)(i * 13U);
    memcpy(expected + 4090, payload, sizeof(payload));
    w25q32_flash_autosector_write_with_erase(payload, 4090, sizeof(payload), 2);
    model_sync();
    assert(w25q32_last_operation_ok() && erases == 2);
    assert(memcmp(memory[1], expected, sizeof(expected)) == 0);
    assert(memory[0][4090] == 0xff);
    w25q32_flash_read(out, 4090, sizeof(out), 2);
    assert(w25q32_last_operation_ok() && memcmp(out, payload, sizeof(out)) == 0);
    before = transactions;
    w25q32_flash_page256_write_no_erase(payload, 255, 2, 2);
    assert(!w25q32_last_operation_ok() && transactions == before);
    w25q32_flash_autopage_write_no_erase(payload, 0x3fffff, 2, 2);
    assert(!w25q32_last_operation_ok() && transactions == before);
    w25q32_flash_autosector_write_with_erase(SPI1_FLASH_BUF, 0, 3, 2);
    assert(!w25q32_last_operation_ok() && transactions == before);
    w25q32_flash_sector4k_erase(1024, 2);
    assert(!w25q32_last_operation_ok() && transactions == before);
    w25q32_flash_read(out, 0, 1, 0);
    assert(!w25q32_last_operation_ok() && transactions == before);
    /* Disconnected MISO reads FF: bounded busy wait, never erase on read failure. */
    detached = true; before = erases; delay_count = 0;
    w25q32_flash_autosector_write_with_erase(payload, 0, 1, 2);
    assert(!w25q32_last_operation_ok() && erases == before);
    assert(delay_count == 3000);
    detached = false; spi_failure = true;
    memset(out, 0, sizeof(out));
    w25q32_flash_read(out, 0, sizeof(out), 2);
    assert(!w25q32_last_operation_ok() && out[0] == 0xff && out[699] == 0xff);
    spi_failure = false; locked = true; wel[1] = false;
    w25q32_flash_autopage_write_no_erase(payload, 0, 1, 2);
    assert(!w25q32_last_operation_ok());
    locked = false;
    w25q32_flash_read(out, 4090, sizeof(out), 2);
    assert(w25q32_last_operation_ok() && memcmp(out, payload, sizeof(out)) == 0);
}

static void test_data(void)
{
    u8 output[4] = {0xaa, 0xbb, 0xcc, 0xdd}, checksum;
    char text[5] = "keep";
    s8 signed_value = 17;
    assert(hex_str_to_bytes_by_len("AB02", 4, output, sizeof(output)) == 2);
    assert(output[0] == 0xab && output[1] == 2 && output[2] == 0xcc);
    assert(hex_str_to_bytes_by_len("12xz", 4, output, sizeof(output)) == 0 && output[0] == 0xab);
    assert(hex_str_to_bytes_by_len("1234", 4, output, 1) == 0);
    assert(!dec_str_to_s8("999999999999", &signed_value) && signed_value == 17);
    assert(dec_str_to_s8("-128", &signed_value) && signed_value == -128);
    assert(!dec_str_to_s8("128", &signed_value));
    assert(!str_intercept("<abcde>", "<", ">", text, sizeof(text)) && strcmp(text, "keep") == 0);
    assert(!str_extract_equal("<abc><abd>", "<", ">", text, sizeof(text)) && strcmp(text, "keep") == 0);
    assert(!str_extract_equal("<abc>", "<", ">", text, sizeof(text)));
    assert(str_extract_equal("<abc><abc>", "<", ">", text, sizeof(text)) && strcmp(text, "abc") == 0);
    assert(str_count_substr("aaaa", "aa") == 2 && str_count_substr("abc", "") == 0);
    assert(xor_checksum_range(output, 0, 1, &checksum) && checksum == (0xab ^ 2));
    assert(!xor_checksum_range(output, 2, 1, &checksum));
}

int main(void)
{
    test_flash();
    test_data();
    puts("PASS: Flash page/sector boundaries, retained data, chip isolation, failure recovery, bounded busy wait, data conversion bounds");
    return 0;
}
