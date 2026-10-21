#ifndef __BLE_APP_H__
#define __BLE_APP_H__
#include "nrf_ble_scan.h"

extern volatile uint8_t           g_adv_wr ;           // 中断里写索引
extern volatile uint8_t           g_adv_rd ;           // 主循环里读索引
//*****************************************************************************************************************
#define APP_BLE_CONN_CFG_TAG    1           // SoftDevice BLE配置标志   
#define ADV_EVT_BUF_SIZE        128          // 最多缓存16个广播事件

typedef struct {
    ble_gap_addr_t  peer_addr;        // 设备地址
    int8_t          rssi;             // 信号强度
    uint8_t         ch_index;         // 信道号
    uint8_t         data_len;         // 广播数据长度
    uint8_t         data[31];         // 广播数据内容
    int8_t          tx_power;         // 广播发射功率
} adv_evt_t;


extern bool run_flag_a  ;                          //  是否运行开始扫描广播数据的标志位，  true = 运行， false = 不运行  
//*****************************************************************************************************************

void  ble_passive_scanning_init (void);  //ble被动扫描初始化
void main_loop_process(void);            //将循环打印接收到的广播数据

#endif



