#ifndef __SYSTICK_DElAY_FREERTOS_H__
#define __SYSTICK_DElAY_FREERTOS_H__
#include "stm32f4xx.h"
#include "FreeRTOS.h"   // 使用freertos 操作系统时候包含头文件
#include "task.h"
//===========================================================================
extern void xPortSysTickHandler(void);
//===========================================================================
void systick_delay_freerots_init(u8 SYSCLK);
void systick_delay_us(u32 nus);
void systick_delay_freerots_ms(u32 nms);
void systick_delay_ms(u32 nms);



#endif


