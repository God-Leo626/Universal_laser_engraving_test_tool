#ifndef __TIEM3_H__
#define __TIEM3_H__
#include "stdbool.h"
#include "stm32f10x.h"
//===========================================================================
extern u32 time3_count1;
extern u32 time3_count2;
extern unsigned char string_buf[100] ; // 用于sprintf函数打印字符串的保存
//===========================================================================
void time3_init(u16 reload,u16 psc);
void time3_run(bool run);


#endif




