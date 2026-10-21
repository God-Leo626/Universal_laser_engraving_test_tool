/**
 * 程序入口：完成硬件/UI初始化，创建启动任务，再交给FreeRTOS调度。
 * 两个业务任务分别处理PC命令和NRF52832数据，详见my_task.c。
 */
#include "ui_lcd.h"
#include "my_task.h"
#include "usart_dma_all.h"

/* 异常停机信息：调试器可直接查看；不在损坏的任务栈上格式化打印。 */
const char * volatile board_fault_reason = NULL;
const char * volatile board_assert_file = NULL;
volatile int board_assert_line = 0;

void board_fatal_error(const char *reason)
{
    __disable_irq();
    board_fault_reason = reason;
    /* 启动失败/栈溢出后禁止继续处理业务，复位后才能重新运行。 */
    for (;;)
    {
        __NOP();
    }
}

void vAssertCalled(const char *file, int line)
{
    __disable_irq();
    board_assert_file = file;
    board_assert_line = line;
    board_fatal_error("FreeRTOS assertion");
}

int main(void)
{
    BaseType_t create_status;

    /* 启动文件的HSE失败分支可能仍运行于HSI，禁止按168MHz误配外设。 */
    SystemCoreClockUpdate();
    if (SystemCoreClock != 168000000UL)
        board_fatal_error("unexpected system clock" );

    /* 此时调度器尚未启动，初始化中使用的延时必须支持裸机阶段。 */
    ui_display_hardware_init();

    /* 栈深度以StackType_t为单位；Cortex-M4上每单位4字节。 */
    create_status = xTaskCreate(start_task, "start_task", START_STK_SIZE,
                               NULL, START_TASK_PRIO, &StartTask_Handler);
    if (create_status != pdPASS)
    {
        usart1_printf("start_[task] create  [>> fail <<]\r\n");
        board_fatal_error("start task allocation failed");
    }

    usart1_printf("start_[task] create  [>> ok <<]\r\n");
    vTaskStartScheduler();

    /* 正常情况下不返回；内核任务分配失败时不能落出main。 */
    board_fatal_error("scheduler start failed");
    return -1;
}