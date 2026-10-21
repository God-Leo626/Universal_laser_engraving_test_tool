#include "my_printf.h"



char print_buf[PRINT_BUF_LEN];
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
* 整包dma发送
* 
*********************************************************************
*********************************************************************/

// 自定义批量打印，直接调用你的DMA发送函数
void print(const char *fmt,...)
{
     uint32_t tx_timeout = 1000000;         // 约 100ms 超时
    va_list ap;
    va_start(ap,fmt);
    vsnprintf(print_buf,PRINT_BUF_LEN,fmt,ap);
    va_end(ap);
    // 调用你之前的EasyDMA发送接口
    uart_dma_send((uint8_t*)print_buf,strlen(print_buf));  
//	while (uart_dma_is_tx_busy()){} ;
    while (uart_dma_is_tx_busy())
    {
        if(--tx_timeout == 0)              // 超时放弃，释放主循环
        {
            break;
        }
    }
}

