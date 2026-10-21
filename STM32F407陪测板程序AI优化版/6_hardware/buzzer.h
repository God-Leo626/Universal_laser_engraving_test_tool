#ifndef __BUZZER_H__ 
#define __BUZZER_H__
#include "stm32f4xx.h" 
#include "io_bit.h"
#include "stdbool.h"


#define BUZZER_CTRL   PEout(2) /* ”Îbuzzer_init≈‰÷√µƒPE2“ª÷¬ */         

//**************************************************************************************
void buzzer_init(void);
void buzzer_ctrl(u16 runtime , bool on_off);


#endif


