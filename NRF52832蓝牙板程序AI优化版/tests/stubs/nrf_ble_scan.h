#ifndef TEST_NRF_BLE_SCAN_H
#define TEST_NRF_BLE_SCAN_H
#include "nrfx_uarte.h"
#define NRF_BLE_SCAN_SCAN_INTERVAL 64
#define NRF_BLE_SCAN_SCAN_WINDOW 64
#define BLE_GAP_PHY_1MBPS 1
#define BLE_GAP_SCAN_FP_ACCEPT_ALL 0
typedef struct { uint8_t addr[6]; } ble_gap_addr_t;
typedef struct {
    ble_gap_addr_t peer_addr;
    int8_t rssi, tx_power;
    uint8_t ch_index;
    struct { uint8_t *p_data; uint16_t len; } data;
} ble_gap_evt_adv_report_t;
typedef struct { uint8_t active; uint16_t interval, window, timeout; uint8_t scan_phys, filter_policy; } ble_gap_scan_params_t;
typedef struct { int unused; } nrf_ble_scan_t;
#define NRF_BLE_SCAN_DEF(name) nrf_ble_scan_t name
typedef struct {
    bool connect_if_match;
    const ble_gap_scan_params_t *p_scan_param;
    uint8_t conn_cfg_tag;
} nrf_ble_scan_init_t;
enum { NRF_BLE_SCAN_EVT_NOT_FOUND, NRF_BLE_SCAN_EVT_SCAN_TIMEOUT };
typedef struct { int scan_evt_id; union { const ble_gap_evt_adv_report_t *p_not_found; } params; } scan_evt_t;
typedef void (*mock_scan_handler_t)(const scan_evt_t *);
int nrf_ble_scan_init(nrf_ble_scan_t *, const nrf_ble_scan_init_t *, mock_scan_handler_t);
int nrf_ble_scan_start(nrf_ble_scan_t *);
#endif
