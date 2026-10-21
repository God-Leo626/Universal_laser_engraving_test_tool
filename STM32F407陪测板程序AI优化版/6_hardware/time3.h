#ifndef __TIEM3_H__
#define __TIEM3_H__
#include "stdbool.h"
#include "stm32f4xx.h"
//===========================================================================

extern volatile bool pc_reback_msg_to_usart1_timeout  ;   // pc返回给串口1的消息超时未返回

//===========================================================================
void time3_init(u16 reload,u16 psc);
void time3_run(bool run);
/* 在LCD所属任务的循环中调用；ISR不再绘屏。 */
void time3_update_display(void);


#endif




