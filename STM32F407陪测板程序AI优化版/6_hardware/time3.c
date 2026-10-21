/* TIM3以500ms为单位计时。中断仅维护状态，绘屏由串口2业务任务调用。 */
#include "time3.h"
#include "lcd.h"
#include "variable_global.h"
#include <stdio.h>

volatile bool pc_reback_msg_to_usart1_timeout = false;
static volatile bool display_pending = false;

void time3_init(u16 reload, u16 psc)
{
    TIM_TimeBaseInitTypeDef timer;
    NVIC_InitTypeDef irq;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_Cmd(TIM3, DISABLE);
    TIM_TimeBaseStructInit(&timer);
    timer.TIM_ClockDivision = TIM_CKD_DIV1;
    timer.TIM_CounterMode = TIM_CounterMode_Up;
    timer.TIM_Period = reload;
    timer.TIM_Prescaler = psc;
    TIM_TimeBaseInit(TIM3, &timer);
    /* TimeBaseInit产生UG更新事件，先清除UIF，防止初始化后立即假计时一次。 */
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    irq.NVIC_IRQChannel = TIM3_IRQn;
    irq.NVIC_IRQChannelPreemptionPriority = 4;
    irq.NVIC_IRQChannelSubPriority = 0;
    irq.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&irq);
}

/* 重启硬件周期，软件累计值仍由业务层在一轮结束后清零。 */
void time3_run(bool run)
{
    TIM_Cmd(TIM3, DISABLE);
    TIM_SetCounter(TIM3, 0);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    display_pending = false;
    if (run) TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        ++time3_count1;
        if ((time3_count1 & 1U) == 0) display_pending = true;
        if (time3_count1 == 10) pc_reback_msg_to_usart1_timeout = true;
    }
}

/* 只由LCD所属任务调用。短暂屏蔽TIM3取快照，恢复后才格式化/绘屏。 */
void time3_update_display(void)
{
    static bool display_flag = false;
    char text[16]; /* [596523:14:07]及结束符也可容纳；原10字节会越界。 */
    u32 count, seconds, h, m, s;
    u32 primask = __get_PRIMASK();
    bool pending;
    __disable_irq();
    pending = display_pending;
    display_pending = false;
    count = time3_count1;
    __set_PRIMASK(primask);
    if (!pending) return;
    seconds = count / 2; /* 避免先乘500导致约49天时溢出。 */
    h = seconds / 3600;
    m = (seconds / 60) % 60;
    s = seconds % 60;
    snprintf(text, sizeof(text), "[%02lu:%02lu:%02lu]", (unsigned long)h,
             (unsigned long)m, (unsigned long)s);
    display_flag = !display_flag;
    lcd_fill_color(210, 440, 318, 477, Black);
    display_ascii_string(224, 450, 8, (u8 *)text,
                         display_flag ? Purple : OrangeRed, Black);
}
