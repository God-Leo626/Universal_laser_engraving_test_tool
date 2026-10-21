#ifndef __LED_H__
#define __LED_H__
#include "io_bit.h"
#include "stdbool.h"
//======================================================================================
#define LED_R  PBout(13)
#define LED_G  PCout(6)
#define LED_B  PCout(8)
//**************************************************************************************
void led_init(void);
void rgb_led_ctrl(bool r,bool g, bool b);

#endif
	
	

