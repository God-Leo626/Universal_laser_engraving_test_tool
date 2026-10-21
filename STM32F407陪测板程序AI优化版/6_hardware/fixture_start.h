#ifndef __FIXTURE_START_H__
#define __FIXTURE_START_H__
#include "stm32f4xx.h"
#include "io_bit.h"
#include "stdbool.h"
//===========================================================================
#define START  PEin(0)       // start PE0
/* 本版治具中断无业务状态；主扫描流程由PC/KEY3命令触发。 */

//===========================================================================
void fixture_start_exti_init(void);
void fixture_start_init_for_calibration(void);
#endif



