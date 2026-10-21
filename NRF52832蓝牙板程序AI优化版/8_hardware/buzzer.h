#ifndef __BUZZER_H__
#define __BUZZER_H__
#include "stdint.h"
#include "stdbool.h"
//*****************************************************************************************************************



#define BUZZER    12

//*****************************************************************************************************************
void buzzer_init(void);
void buzzer_ctrl(bool on_off, uint16_t time);



#endif


