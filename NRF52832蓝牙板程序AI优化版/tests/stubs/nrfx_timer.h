#ifndef TEST_NRFX_TIMER_H
#define TEST_NRFX_TIMER_H
#include "nrfx_uarte.h"
typedef enum { NRF_TIMER_EVENT_COMPARE0 } nrf_timer_event_t;
typedef enum { NRF_TIMER_TASK_CLEAR, NRF_TIMER_TASK_START, NRF_TIMER_TASK_CAPTURE1 } nrf_timer_task_t;
typedef enum { NRF_TIMER_CC_CHANNEL0, NRF_TIMER_CC_CHANNEL1 } nrf_timer_cc_channel_t;
typedef void (*nrfx_timer_event_handler_t)(nrf_timer_event_t, void *);
typedef struct { uint32_t cc[2]; } mock_timer_t;
extern mock_timer_t mock_timer;
#define NRF_TIMER1 (&mock_timer)
static inline void nrf_timer_cc_write(mock_timer_t *timer, nrf_timer_cc_channel_t ch, uint32_t value) { timer->cc[ch] = value; }
static inline uint32_t nrf_timer_cc_read(mock_timer_t *timer, nrf_timer_cc_channel_t ch) { return timer->cc[ch]; }
static inline void nrf_timer_event_clear(mock_timer_t *timer, nrf_timer_event_t evt) { (void)timer; (void)evt; }
#endif
