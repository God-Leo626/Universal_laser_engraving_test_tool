#ifndef __TIEM3_H__
#define __TIEM3_H__
#include "stdbool.h"
#include "stm32f4xx.h"
//===========================================================================

extern bool pc_reback_msg_to_usart1_timeout  ;   // pc返回给串口1的消息超时未返回

//===========================================================================
void time3_init(u16 reload,u16 psc);
void time3_run(bool run);


#endif




