

#include <stdio.h>

#include "nrfx_config.h"
#include "nrfx_uarte.h" // UARTE - EASYDMA 核心驱动
#include "nrfx.h "
//#include "nrf_52832_uarte.h"
// 全局UARTE实例

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-02-16
* 举例：
* 备注：
* 
* 单字节DMA发送
*********************************************************************
*********************************************************************/

// 单字符发送（DMA阻塞等待发完）
//static void uarte_putc(uint8_t ch)
//{
//	static uint8_t temp_ch; //静态缓存，避免栈变量DMA不稳定
//    temp_ch = ch;
//    nrfx_uarte_tx(&m_uarte, &temp_ch, 1);
//    while(nrfx_uarte_tx_in_progress(&m_uarte)); //等待DMA发送结束
//}

//// printf底层重定向：Keil MDK标准fputc
//int fputc(int ch, FILE *f)
//{
//    if(f == stdout)
//    {
//        uarte_putc((uint8_t)ch);
//        // 自动换行优化：'\n'补\r
//        if(ch == '\n')
//		{
//			uarte_putc('\r');
//		}			
//    }
//    return ch;
//}


int fputc(int ch, FILE *f)
{
    // 直接操作 UART 寄存器发送，不初始化、不冲突
    while (NRF_UART0->EVENTS_TXDRDY == 0);
    NRF_UART0->EVENTS_TXDRDY = 0;
    NRF_UART0->TXD = (uint8_t)ch;
    return ch;
}
