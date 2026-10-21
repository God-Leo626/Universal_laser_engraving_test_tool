#ifndef   __TIME1_H__
#define   __TIME1_H__
#include "stm32f4xx.h"


void Timer1_Ch3_Output_PWM(u16 psc,u16 arr,u16 Compare);// psc=1680 , arr=100 T=(psc*arr)/168M =1ms

#endif


