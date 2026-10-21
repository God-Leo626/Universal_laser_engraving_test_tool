#include "timer.h"
#include "app_error.h"

static const nrfx_timer_t m_timer = NRFX_TIMER_INSTANCE(1);

void timer1_init(uint32_t timeout_us, uint8_t irq_priority, nrfx_timer_event_handler_t handler)
{
    nrfx_timer_config_t config = NRFX_TIMER_DEFAULT_CONFIG;
    config.frequency = NRF_TIMER_FREQ_1MHz; /* 1 tick = 1 us。 */
    config.mode = NRF_TIMER_MODE_TIMER;
    config.bit_width = NRF_TIMER_BIT_WIDTH_32;
    config.interrupt_priority = irq_priority;
    config.p_context = NULL;
    /* 未初始化时不能 uninit，否则 SDK 的状态断言会失败。 */
    APP_ERROR_CHECK(nrfx_timer_init(&m_timer, &config, handler));
    nrfx_timer_compare(&m_timer, NRF_TIMER_CC_CHANNEL0,
                      nrfx_timer_us_to_ticks(&m_timer, timeout_us), true);
    timer1_stop_and_clear();
}

void timer1_stop(void)
{
    /* 由 PPI 启动计数，使用 STOP 而非 SHUTDOWN，保留捕获通道状态。 */
    nrfx_timer_pause(&m_timer);
}

void timer1_clear(void)
{
    nrfx_timer_clear(&m_timer);
}

void timer1_stop_and_clear(void)
{
    timer1_stop();
    timer1_clear();
}

uint32_t timer1_task_addr_get(nrf_timer_task_t task)
{
    return nrfx_timer_task_address_get(&m_timer, task);
}
