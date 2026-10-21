#ifndef __TIMER_H__
#define __TIMER_H__
#include "stdint.h"

#include <nrfx_timer.h> 
//*****************************************************************************************************************
#define TIMER0    0
#define TIMER1    1
#define TIMER2    2
#define TIMER3    3
#define TIMER4    4

//*****************************************************************************************************************
extern const nrfx_timer_t TIMER_INSTANCE ;
void timer1_init(uint32_t timeout_us,uint8_t  irq_priority,nrfx_timer_event_handler_t  handler);
void timer1_stop(void);
void timer1_clear(void);
void timer1_stop_and_clear(void);
uint32_t timer1_task_addr_get(nrf_timer_task_t task);
void timer1_irq_handler(void);



#endif



