#ifndef  __BUTTON_H__
#define  __BUTTON_H__
#include "stdint.h"

#define BUTTON_13   13
#define BUTTON_14   14
#define BUTTON_15   15
#define BUTTON_16   16

//*****************************************************************************************************************
void button_init(void);
void button_ctrl(uint8_t button_num,uint8_t led_num);


#endif


