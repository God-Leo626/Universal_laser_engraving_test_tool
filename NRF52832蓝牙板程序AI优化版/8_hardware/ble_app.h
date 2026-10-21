#ifndef BLE_APP_H
#define BLE_APP_H

/* 初始化 SoftDevice、被动扫描参数并立即开始硬件扫描。 */
void ble_passive_scanning_init(void);
/* 主循环：出队一个报告，过滤、解码并输出。 */
void main_loop_process(void);
/* 配置阶段丢弃已收到的广播，避免旧数据占满队列。 */
void ble_discard_pending_reports(void);
#endif
