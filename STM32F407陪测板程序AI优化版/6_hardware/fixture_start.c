/* 治具输入PE0：低电平表示下压。当前陪测主流程没有启用此通道。
 * 保留初始化接口供旧功能使用；本版中断没有业务动作，仅清标志返回。 */
#include "fixture_start.h"

void fixture_start_init_for_calibration(void)
{
    GPIO_InitTypeDef gpio;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_IN;
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_PuPd = GPIO_PuPd_UP;
    gpio.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOE, &gpio);
}

void fixture_start_exti_init(void)
{
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef irq;
    fixture_start_init_for_calibration();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
    exti.EXTI_Line = EXTI_Line0;
    exti.EXTI_LineCmd = ENABLE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_ClearITPendingBit(EXTI_Line0);
    EXTI_Init(&exti);
    irq.NVIC_IRQChannel = EXTI0_IRQn;
    irq.NVIC_IRQChannelCmd = ENABLE;
    irq.NVIC_IRQChannelPreemptionPriority = 5;
    irq.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&irq);
}

void EXTI0_IRQHandler(void)
{
    /* 原中断没有执行动作，不再在中断内空等15ms或调用RTOS。 */
    EXTI_ClearITPendingBit(EXTI_Line0);
}
