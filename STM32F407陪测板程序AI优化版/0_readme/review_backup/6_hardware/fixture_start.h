#ifndef __FIXTURE_START_H__
#define __FIXTURE_START_H__
#include "io_bit.h"
#include "stdbool.h"
//===========================================================================
#define START  PEin(0)       // start PE0
extern bool fixture_status_flag ; // 治具状态保存  false ： 治具打开  true： 下压到位

//===========================================================================
void fixture_start_exti_init(void);
void fixture_start_init_for_calibration(void);
#endif



