#ifndef __BUZZER_H__ 
#define __BUZZER_H__
#include "stm32f4xx.h" 
#include "io_bit.h"
#include "stdbool.h"


#define BUZZER_CTRL   PCout(10)         

//**************************************************************************************
void buzzer_init(void);
void buzzer_ctrl(u16 runtime , bool on_off);


#endif


