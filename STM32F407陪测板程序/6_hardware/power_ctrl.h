#ifndef __POWER_CTRL_H__
#define __POWER_CTRL_H__
#include "io_bit.h"
#include "stdbool.h"   //使用bool值得头文件
//======================================================================================
//   PE3  3V3 (H : ON   L: OFF)

#define POWER_3V3    PEout(3)




//**************************************************************************************
void power_ctrl_init(void);
void power_ctrl_nrf52832(bool on_off);

//======================================================================================
#endif




