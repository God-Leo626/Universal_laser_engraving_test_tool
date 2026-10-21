#ifndef __BUTTON_H__
#define __BUTTON_H__
#include "stm32f4xx.h"
#include "io_bit.h"
#include "stdbool.h"
//**************************************************************************************
//  PB15 :KEY1  (L:按下   H :没有按下)
//  PB14: KEY2  (L:按下   H :没有按下)
//  PC7  :KEY3  (L:按下   H :没有按下)

#define KEY1   PBin(15)
#define KEY2   PBin(14)
#define KEY3   PCin(7)

#define KEY1_SELECT  1
#define KEY2_SELECT  2
#define KEY3_SELECT  3

extern bool cmd_set_flag_from_buttom  ;  // 由按键任务处理置位，通知使用保存配置
extern  bool cmd_select_flag  ;          // 在key2 中传递信息
extern bool decoding_setting_flag ;   //  在中断里面按一次取反一次， false = 通知nrf52832 按照异或方式解析数据， true = 按照明文方式接续数据
//**************************************************************************************
void button_pin_init(void);
/* 仅由串口2业务任务周期调用；保持20ms按下/释放消抖。 */
void button_process_events(void);
/* 在等待NRF应答期间仅采样锁存，返回主循环后再执行，避免插入新命令。 */
void button_sample_events(void);
void key3_button_pin_init(void);





#endif



