#ifndef __LED_H__
#define __LED_H__
#include "io_bit.h"
#include "stdbool.h"
//======================================================================================
#define LED  PBout(0)

//**************************************************************************************
void led_init(void);
void led_ctrl(bool on_off);

#endif
	
	

