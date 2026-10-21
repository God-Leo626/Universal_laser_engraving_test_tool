/**
 * 历史 printf 重定向示例，未加入当前 Keil 工程。
 *
 * 应用串口输出统一使用 my_printf.h 中的 print()。
 * 旧 fputc 直接等待 UART0->EVENTS_TXDRDY，与当前 UARTE0 EasyDMA
 * 模式不兼容，误加入工程可能永久等待，因此已移除该实现。
 * SDK 的 SEGGER_RTT_Syscalls_KEIL.c 负责工程中标准库的重定向。
 */
