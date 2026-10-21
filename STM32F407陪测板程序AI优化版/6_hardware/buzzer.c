/** 蜂鸣器：PE2输出5kHz方波；保持原100us高/低脉宽。 */
#include "buzzer.h"
#include "systick_delay_freertos.h"

void buzzer_init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_OType = GPIO_OType_PP;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOE, &gpio);
    BUZZER_CTRL = 0;
}

/* runtime单位为ms。忙等输出，请仅从初始化或任务调用，不在中断使用。 */
void buzzer_ctrl(u16 runtime, bool on_off)
{
    u32 pulse;
    const u32 pulse_count = (u32)runtime * 5U;

    if (on_off)
    {
        /* 用32位计数，避免长鸣参数超过13107ms时16位循环变量回绕。 */
        for (pulse = 0; pulse < pulse_count; ++pulse)
        {
            BUZZER_CTRL = 1;
            systick_delay_us(100);
            BUZZER_CTRL = 0;
            systick_delay_us(100);
        }
    }
    BUZZER_CTRL = 0;
}