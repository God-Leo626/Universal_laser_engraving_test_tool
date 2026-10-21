#ifndef __TIEM4_H__
#define __TIEM4_H__
#include "stdbool.h"
#include "stm32f10x.h"
//===========================================================================

extern u32 time4_count2;
//===========================================================================
void time4_init(u16 reload,u16 psc);
void time4_run(bool run);


#endif




