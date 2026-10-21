#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "ble_app.h"
#include "app_config.h"
#include "data_processing.h"
#include "nrf_ble_scan.h"

static unsigned checks;
#define CHECK(expr) do { ++checks; if (!(expr)) { fprintf(stderr, "BLE FAIL line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
uint32_t mock_ipsr;
static mock_scan_handler_t callback;
static unsigned init_step, scan_starts, records;
static char output[1024];
static size_t output_length;
int nrf_sdh_enable_request(void) { CHECK(init_step++ == 0); return 0; }
int nrf_sdh_ble_default_cfg_set(uint8_t tag, uint32_t *ram) { CHECK(init_step++ == 1 && tag == 1); *ram = 0x20002218; return 0; }
int nrf_sdh_ble_enable(uint32_t *ram) { CHECK(init_step++ == 2 && *ram == 0x20002218); return 0; }
int nrf_ble_scan_init(nrf_ble_scan_t *scan, const nrf_ble_scan_init_t *config, mock_scan_handler_t handler)
{
    (void)scan;
    CHECK(init_step++ == 3 && !config->connect_if_match);
    CHECK(config->p_scan_param->active == 0 && config->p_scan_param->interval == 64);
    CHECK(config->p_scan_param->window == 64 && config->p_scan_param->timeout == 0);
    callback = handler; return 0;
}
int nrf_ble_scan_start(nrf_ble_scan_t *scan) { (void)scan; ++scan_starts; return 0; }
void print(const char *format, ...)
{
    if (strncmp(format, "[RSSI=", 6) == 0) { ++records; output_length = 0; }
    va_list args; va_start(args, format);
    int count = vsnprintf(output + output_length, sizeof(output) - output_length, format, args);
    va_end(args); CHECK(count >= 0 && (size_t)count < sizeof(output) - output_length);
    output_length += (size_t)count;
}
static void report(uint8_t *data, uint16_t length)
{
    ble_gap_evt_adv_report_t adv = {0};
    for (unsigned i = 0; i < 6; ++i) adv.peer_addr.addr[i] = (uint8_t)i;
    adv.rssi = -38; adv.tx_power = 127; adv.ch_index = 37;
    adv.data.p_data = data; adv.data.len = length;
    scan_evt_t event = {0}; event.scan_evt_id = NRF_BLE_SCAN_EVT_NOT_FOUND; event.params.p_not_found = &adv;
    callback(&event);
}
int main(void)
{
    uint8_t data[300]; for (unsigned i = 0; i < sizeof(data); ++i) data[i] = (uint8_t)i;
    ble_passive_scanning_init(); CHECK(init_step == 4 && scan_starts == 1);
    g_app_config.rssi_configured = true; g_app_config.rssi_threshold = -60;
    g_app_config.default_mode = true; g_app_config.plaintext = true; g_app_config.show_raw = true;
    report(data, 31); memset(data, 0xFF, 31); main_loop_process();
    CHECK(records == 1);
    CHECK(strstr(output, "[MAC=0X00 01 02 03 04 05 ]") != NULL);
    CHECK(strstr(output, "[DECRYPT_MAC]=08090A0B0C0D0E0F1011121314151617") != NULL);
    CHECK(strstr(output, "[DECRYPT_SN]=18191A1B1C1D1E\r\n") != NULL);
    for (unsigned i = 0; i < sizeof(data); ++i) data[i] = (uint8_t)i;
    report(data, 32); report(data, 300); report(NULL, 31);
    main_loop_process(); CHECK(records == 1);
    for (unsigned i = 0; i < 150; ++i) report(data, 31);
    for (unsigned i = 0; i < 150; ++i) main_loop_process();
    CHECK(records == 128); /* 127 queue slots, no overflow or duplicate consumption. */
    for (unsigned round = 0; round < 4; ++round) {
        for (unsigned i = 0; i < 100; ++i) report(data, 31);
        for (unsigned i = 0; i < 100; ++i) main_loop_process();
    }
    CHECK(records == 528); /* Ring wraps repeatedly. */
    report(data, 31); ble_discard_pending_reports(); main_loop_process(); CHECK(records == 528);
    g_app_config.reverse_address = true; report(data, 31); main_loop_process();
    CHECK(strstr(output, "[MAC=0X05 04 03 02 01 00 ]") != NULL);
    g_app_config.show_raw = false; report(data, 10); main_loop_process();
    CHECK(strcmp(output, "[RSSI=-38] ---> [CH=37] ---> [tx_pwr=127] ---> \r\n") == 0);
    scan_evt_t timeout = {0}; timeout.scan_evt_id = NRF_BLE_SCAN_EVT_SCAN_TIMEOUT; callback(&timeout);
    CHECK(scan_starts == 2);
    printf("BLE queue/output regression PASS: %u checks\n", checks);
    return 0;
}
