#ifndef __UARTE_52832_DMA_H__
#define __UARTE_52832_DMA_H__

#include "stdint.h"
#include <nrfx_uarte.h>
//*****************************************************************************************************************
// 定义nrf52832 串口的pin 
#define RX_PIN_NUMBER  8
#define TX_PIN_NUMBER  6
#define CTS_PIN_NUMBER 7
#define RTS_PIN_NUMBER 5

#define UART_DMA_BAUDRATE        NRF_UARTE_BAUDRATE_115200      // 串口波特率 
#define UART_RX_TIMEOUT_US       (2000)                         // 帧超时（μs）= 2 ms 
#define UART_RX_BUF_SIZE         (255)                          // 接收区缓冲大小
#define RSSI_DEFAULT_VALUE        -60                           //  定义默认的rssi管控阈值
  
extern uint8_t char_global_buf[UART_RX_BUF_SIZE + 7]  ;         // 保存uart接收到的数据
extern volatile bool               start_advertising_flag ;     // 开始广播的标志， true = 开始扫描， 否则不扫描

extern bool rssi_change_result  ;           //  rssi设置结果   true 设置成功
extern bool feature_change_result  ;        //  特征值设置结果  true 设置成功
extern bool byte_order_flag  ;              // 字节序是否逆序输出标志  false = 默认顺序， true= 逆序
extern long rssi_buf  ;                     //  保存设置的rssi值
extern bool run_flag_b  ;                   //  保存使用默认配置开始扫描时候的标志 ，true = 使用默认设置    false= 自定义设置
extern bool  mask_raw_data_global_flag  ;   //    屏蔽扫描接受到的原始数据（ture = 原始数据不通过串口显示  false= 显示原始数据）
extern bool  decoding_set_global_flag  ;    //  对ble数据的解密方式设置 false = 异或解密ble数据  true = 明文解析ble数据 （通过STM32的按键设置）
//*****************************************************************************************************************
typedef enum
{
    UART_DMA_OK                = 0, /**< 成功 */
    UART_DMA_ERR_BUSY          = 1, /**< 资源忙（TX 未完成） */
    UART_DMA_ERR_INVALID_PARAM = 2, /**< 参数无效（NULL、长度 0 等） */
    UART_DMA_ERR_INVALID_STATE = 3  /**< 状态错误 */
} uart_dma_err_t;


//*****************************************************************************************************************
void  uarte_dma_init(void);
uart_dma_err_t uart_dma_send(uint8_t const * p_data, uint16_t length) ;
bool uart_dma_is_tx_busy(void)  ;
bool   uart_dma_rx_ready(void)  ;
uint8_t * uart_dma_rx_data(void);
size_t uart_dma_rx_len(void) ;
void uart_dma_rx_done(void)  ;
void uart_dma_uarte_irq_handler(void)  ;
nrfx_err_t uarte_dma_send(uint8_t const * p_data, uint16_t length);
void uart_data_processing (void);



#endif


