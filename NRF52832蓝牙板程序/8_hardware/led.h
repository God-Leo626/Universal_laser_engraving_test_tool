#ifndef __LED_H__
#define __LED_H__

#include "stdbool.h"
#include "stdint.h"
//*****************************************************************************************************************
#define LED_17    17
#define LED_18    18
#define LED_19    19
#define LED_20    20
#define LED_04    4
//*****************************************************************************************************************
void led_init(void);
void led_ctrl(bool on,uint8_t pin_num);
void led_toggle(uint8_t pin_num);
#endif




