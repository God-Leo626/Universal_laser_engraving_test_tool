#ifndef __RELAY_H__ 
#define __RELAY_H__ 

#include "stm32f4xx.h" 
#include "io_bit.h"
#include "stdbool.h"
//*****************************************************************************************************************
extern	u8 multiplier_global  ;
// extern u8 range_changeover_times_global   ;    // 用于标记量程切换的次数，管控切换
extern u8  test_level_select_falg   ;            // 用于标记量程切换的标记控制
// H :导通
// L ：不导通
#define CH1           0   // 表示通道1
#define CH2           1   // 表示通道2
#define CH3           2   // 表示通道3
#define CH4           3   // 表示通道4

#define PA0_0          0   // PA0 通道0   (ADC 采集引脚 +通道)
#define PA1_1          1   // PA1 通道1   (ADC 采集引脚 +通道)
#define PA2_2          2   // PA2 通道2   (ADC 采集引脚 +通道)
#define PC3_13         3   // PC3 通道13  (ADC 采集引脚 +通道)

#define SHUNT_RESISTOR_0_02R   0   // 检流电阻0.02R 
#define SHUNT_RESISTOR_5R      1   // 检流电阻0.02R 
#define SHUNT_RESISTOR_300R    2   // 检流电阻0.02R 


#define H       0   // 表示选择 高 电流测试档位
#define M       1   // 表示选择 中 电流测试档位
#define L       2   // 表示选择 低 电流测试档位

#define RELAY_K1    PCout(11)
#define RELAY_K2    PCout(12)




//*****************************************************************************************************************
void relay_init(void);
void relay_ctrl(u8 channel_num,u8 level_select);
void display_current(u8 read_num,u8 delete_num);
void zero_calibration(u8 read_num,u8 delete_num);
double get_average_current(u8 read_num,u8 delete_num) ;
double auto_get_adc_average_current(u8 read_num,u8 delete_num,u8 adc_select,u8 shunt_resistor_select) ;
#endif



