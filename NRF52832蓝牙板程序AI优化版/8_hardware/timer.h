#ifndef APP_TIMER1_H
#define APP_TIMER1_H
#include <stdint.h>
#include "nrfx_timer.h"

/* TIMER1 CC0: UART 空闲检测；CC1: RXTO 停止完成捕获。只初始化一次。 */
void timer1_init(uint32_t timeout_us, uint8_t irq_priority, nrfx_timer_event_handler_t handler);
void timer1_stop(void);
void timer1_clear(void);
void timer1_stop_and_clear(void);
uint32_t timer1_task_addr_get(nrf_timer_task_t task);
#endif
