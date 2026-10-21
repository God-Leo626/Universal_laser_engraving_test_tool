/**
 * BLE 应用层：回调只校验并入队；主循环负责过滤、解码和串口输出。
 * 被动扫描、不发起连接、不去重，保持原来的采集用途与参数。
 */
#include "ble_app.h"
#include "adv_parser.h"
#include "app_config.h"
#include "my_printf.h"
#include "nrf_ble_scan.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_error.h"
#include <string.h>

#define APP_BLE_CONN_CFG_TAG 1u
#define ADV_EVT_BUF_SIZE 128u /* 环形队列空出一格用于判满，可存 127 条。 */

typedef struct {
    uint8_t address[6];
    int8_t rssi;
    uint8_t channel;
    int8_t tx_power;
    uint8_t length;
    uint8_t data[ADV_DATA_MAX_LENGTH];
} adv_evt_t;

NRF_BLE_SCAN_DEF(m_scan);
static adv_evt_t m_reports[ADV_EVT_BUF_SIZE];
static volatile uint8_t m_write;
static volatile uint8_t m_read;
/* 可在调试器查看；队列满时保持原来的丢弃新报告策略。 */
static volatile uint32_t m_dropped_reports;

static const ble_gap_scan_params_t m_scan_params = {
    .active = 0,
    .interval = NRF_BLE_SCAN_SCAN_INTERVAL, /* 当前 sdk_config：64 * 0.625 = 40 ms。 */
    .window = NRF_BLE_SCAN_SCAN_WINDOW,     /* 当前 sdk_config：64 * 0.625 = 40 ms。 */
    .timeout = 0,
    .scan_phys = BLE_GAP_PHY_1MBPS,
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL
};

static uint8_t next_index(uint8_t index)
{
    return (uint8_t)((index + 1u) % ADV_EVT_BUF_SIZE);
}

static void scan_start(void)
{
    APP_ERROR_CHECK(nrf_ble_scan_start(&m_scan));
}

static void scan_evt_handler(const scan_evt_t *event)
{
    if (event == NULL) return;
    if (event->scan_evt_id == NRF_BLE_SCAN_EVT_NOT_FOUND) {
        const ble_gap_evt_adv_report_t *report = event->params.p_not_found;
        uint8_t next = next_index(m_write);
        adv_evt_t *slot;
        /* 先检查，后 memcpy；不能把越界判断延迟到主循环。 */
        if (report == NULL || report->data.len > ADV_DATA_MAX_LENGTH ||
            (report->data.len != 0 && report->data.p_data == NULL)) {
            ++m_dropped_reports;
            return;
        }
        if (next == m_read) {
            ++m_dropped_reports;
            return;
        }
        __DMB();
        slot = &m_reports[m_write];
        memcpy(slot->address, report->peer_addr.addr, sizeof(slot->address));
        slot->rssi = report->rssi;
        slot->channel = report->ch_index;
        slot->tx_power = report->tx_power;
        slot->length = (uint8_t)report->data.len;
        if (slot->length != 0) memcpy(slot->data, report->data.p_data, slot->length);
        /* 单生产者（BLE 回调）、单消费者（主循环）；写完再发布索引。 */
        __DMB();
        m_write = next;
    } else if (event->scan_evt_id == NRF_BLE_SCAN_EVT_SCAN_TIMEOUT) {
        scan_start();
    }
}

static bool report_pop(adv_evt_t *report)
{
    if (m_read == m_write) return false;
    __DMB();
    *report = m_reports[m_read];
    __DMB();
    m_read = next_index(m_read);
    return true;
}

void ble_discard_pending_reports(void)
{
    uint8_t write = m_write;
    /* 只移动读指针到已发布的快照；并发到达的新报告留给下一次循环。 */
    __DMB();
    m_read = write;
}

static void print_hex(const uint8_t *data, size_t length)
{
    for (size_t i = 0; i < length; ++i) print("%02X", data[i]);
}

static void print_raw_report(const adv_evt_t *report)
{
    print("[MAC=0X");
    for (size_t i = 0; i < sizeof(report->address); ++i) {
        size_t index = g_app_config.reverse_address ? sizeof(report->address) - 1u - i : i;
        print("%02X ", report->address[index]);
    }
    print("] ");
    if (report->length != 0) {
        print("---> [DATA=0X");
        for (size_t i = 0; i < report->length; ++i) print("%02X ", report->data[i]);
        print("]");
    }
    print("\r\n");
}

void main_loop_process(void)
{
    adv_evt_t report;
    adv_fields_t fields;
    if (!report_pop(&report)) return;
    /* 使用局部副本，缓慢打印时不再占着队列槽或共享解码缓冲。 */
    if (!adv_matches(report.data, report.length, report.rssi, &g_app_config)) return;
    print("[RSSI=%d] ---> [CH=%d] ---> [tx_pwr=%d] ---> ",
          report.rssi, report.channel, report.tx_power);
    if (g_app_config.show_raw) print_raw_report(&report);
    if (adv_decode_fields(report.data, report.length, g_app_config.plaintext, &fields)) {
        if (fields.mac_length != 0) {
            print("[DECRYPT_MAC]=");
            print_hex(fields.mac, fields.mac_length);
        }
        if (fields.sn_length != 0) {
            print(" ---> [DECRYPT_SN]=");
            print_hex(fields.sn, fields.sn_length);
        }
        print("\r\n");
    } else if (!g_app_config.show_raw) {
        /* 无法解码也结束本行，避免下一条广播和本行粘连。 */
        print("\r\n");
    }
}

void ble_passive_scanning_init(void)
{
    uint32_t ram_start = 0;
    nrf_ble_scan_init_t scan = {0};
    APP_ERROR_CHECK(nrf_sdh_enable_request());
    APP_ERROR_CHECK(nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start));
    APP_ERROR_CHECK(nrf_sdh_ble_enable(&ram_start));
    scan.connect_if_match = false;
    scan.p_scan_param = &m_scan_params;
    scan.conn_cfg_tag = APP_BLE_CONN_CFG_TAG;
    APP_ERROR_CHECK(nrf_ble_scan_init(&m_scan, &scan, scan_evt_handler));
    scan_start();
}
