/* SysTick与FreeRTOS共用HCLK计数源；短延时轮询VAL，不重配运行中的时钟。
 * systick_delay_ms是忙等；需要让出CPU的任务使用systick_delay_freerots_ms。 */
#include "systick_delay_freertos.h"

static u32 ticks_per_us = 0;

void SysTick_Handler(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
        xPortSysTickHandler();
}

void systick_delay_freerots_init(u8 SYSCLK)
{
    u32 reload;
    ticks_per_us = SYSCLK;
    reload = ((u32)SYSCLK * 1000000UL) / configTICK_RATE_HZ;
    if (reload == 0 || reload > 0x1000000UL) return;
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
    SysTick->LOAD = reload - 1; /* 计数周期为LOAD+1。 */
    SysTick->VAL = 0;
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}

void systick_delay_us(u32 nus)
{
    if (nus == 0 || ticks_per_us == 0) return;
    /* 分段计算避免nus * 主频溢出。依赖SysTick持续运行。 */
    while (nus != 0)
    {
        u32 part = nus > 1000000UL ? 1000000UL : nus;
        u32 target = part * ticks_per_us;
        u32 elapsed = 0;
        u32 previous = SysTick->VAL;
        u32 period = SysTick->LOAD + 1;
        while (elapsed < target)
        {
            u32 current = SysTick->VAL;
            elapsed += current <= previous ? previous - current : period - current + previous;
            previous = current;
        }
        nus -= part;
    }
}

void systick_delay_ms(u32 nms)
{
    while (nms != 0) { systick_delay_us(1000); --nms; }
}

void systick_delay_freerots_ms(u32 nms)
{
    /* 中断、调度器未启动/暂停时不能vTaskDelay，改用原来的忙等路径。 */
    if (__get_IPSR() != 0 || xTaskGetSchedulerState() != taskSCHEDULER_RUNNING)
    {
        systick_delay_ms(nms);
        return;
    }
    while (nms != 0)
    {
        u32 part = nms > 60000UL ? 60000UL : nms;
        TickType_t ticks = (TickType_t)(((uint64_t)part * configTICK_RATE_HZ) / 1000);
        u32 consumed = (u32)(((uint64_t)ticks * 1000) / configTICK_RATE_HZ);
        if (ticks != 0) vTaskDelay(ticks);
        systick_delay_ms(part - consumed);
        nms -= part;
    }
}
