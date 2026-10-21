#ifndef APP_CONFIG_H
#define APP_CONFIG_H
#include <stdbool.h>
#include <stdint.h>

#define RSSI_DEFAULT_VALUE (-60)
#define RSSI_MIN_VALUE (-105)
#define RSSI_MAX_VALUE 0

/** 所有配置仅在主循环修改；开始输出后锁定，复位恢复初始值。 */
typedef struct {
    bool scanning_started;  /* 控制业务输出；射频扫描在初始化时已启动。 */
    bool default_mode;      /* true: 仅 RSSI 过滤；false: RSSI + 特征前缀。 */
    bool rssi_configured;
    int8_t rssi_threshold;
    bool reverse_address;   /* 仅影响原始广播地址显示，不反转解码字段。 */
    bool show_raw;          /* 显示原始广播地址和 DATA。 */
    bool plaintext;         /* false: 原协议 XOR；true: 固定偏移明文。 */
} app_config_t;

extern app_config_t g_app_config;
#endif
