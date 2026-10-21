#ifndef __POWER_CTRL_H__
#define __POWER_CTRL_H__
#include "io_bit.h"
#include "stdbool.h"   //使用bool值得头文件
//======================================================================================
//   PD6  3V3 (H : ON   L: OFF)
//   PC12  5V (H : ON   L: OFF)
#define POWER_3V3    PDout(6)
#define POWER_5V     PCout(12)

#define PWR_3V3_SELECT   3
#define PWR_5V_SELECT    5
//**************************************************************************************
void power_ctrl_init(void);
void power_ctrl_ext(u8 pwr_select,bool on_off);

//======================================================================================
#endif




