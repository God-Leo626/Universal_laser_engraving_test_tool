#ifndef __BUTTON_H__
#define __BUTTON_H__
#include "stm32f10x.h"
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

//**************************************************************************************
void button_pin_init(void);





#endif



