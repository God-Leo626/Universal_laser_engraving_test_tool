#include "uarte_52832_dma.h"
#include "ppi.h" 
#include "timer.h"
#include <hal/nrf_timer.h> 
#include <hal/nrf_uarte.h> 
#include <nrfx_uarte.h>
#include "nrf_gpio.h"
#include "led.h"
#include "nrf_delay.h"
#include "string.h"
#include "my_printf.h"
#include "data_processing.h"
#include "ble_app.h"
//*****************************************************************************************************************
static nrfx_uarte_t         m_uarte = NRFX_UARTE_INSTANCE(0);          /* UARTE0 实例 */
__ALIGN(4) static uint8_t   m_rx_buf[UART_RX_BUF_SIZE];                /* 内部接收缓冲（256 字节，RAM 中） */



static volatile bool        m_rx_ready = false;                  /* 接收完成标志：true = 有新帧 */
static volatile size_t      m_rx_len   = 0;                      /* 当前帧的字节数 */
static volatile bool        m_tx_busy  = false;                  /* TX 发送中标志 */
volatile bool               start_advertising_flag = false; // 开始广播的标志， true = 开始扫描， 否则不扫描
                                             
uint8_t char_global_buf[UART_RX_BUF_SIZE + 7]  = {0} ;// 保存uart接收到的数据

bool rssi_change_result = false ;       //  rssi设置结果   true 设置成功
bool feature_change_result = false ;    //  特征值设置结果  true 设置成功
bool byte_order_flag = false ;          //  字节序是否逆序输出标志  false = 默认顺序， true= 逆序
long rssi_buf = 0 ;                     //  保存设置的rssi值
bool run_flag_b = false ;                   //  保存使用默认配置开始扫描时候的标志 ，true = 使用默认设置    false= 自定义设置
bool  mask_raw_data_global_flag = false ;  //  屏蔽扫描接受到的原始数据（ture = 原始数据通过串口显示  false= 显示原始数据不显示）
bool  decoding_set_global_flag = false ;   //  对ble数据的解密方式设置 false = 异或解密ble数据  true = 明文解析ble数据 （通过STM32的按键设置）
//*****************************************************************************************************************




/*******************************************************************
********************************************************************
* 名称：
* 功能： 启动 UARTE 接收（EasyDMA）
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 使用 HAL 直接设置接收缓冲、清除事件、触发 STARTRX，并使能中断。
 * 不经过 nrfx_uarte_rx()，以维持对接收流程的完全控制。
*********************************************************************
*********************************************************************/
static void uart_dma_rx_start(void)
{	
  
	nrfx_uarte_rx(&m_uarte, m_rx_buf, UART_RX_BUF_SIZE);  /* nrfx 官方 API 启动接收，内部正确写入 RXD.PTR 和 RXD.MAXCNT */
	nrf_uarte_int_disable(NRF_UARTE0, NRF_UARTE_INT_ENDRX_MASK);  /* 关闭 ENDRX 中断（我们不靠缓冲满检测帧结束） */
					
}		 
/*******************************************************************
********************************************************************
* 名称：
* 功能：nrfx_uarte 事件回调
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
*   rxd 的硬件fifo有6个字节
*********************************************************************
*********************************************************************/
static void uarte_evt_handler(nrfx_uarte_event_t const * p_event, void * p_context)
{
    (void)p_context;                                        /* 不使用上下文 */
    switch (p_event->type)
    {
		case NRFX_UARTE_EVT_TX_DONE:                            /* 发送完成 */
        {
            m_tx_busy = false;                                  /* 清除发送忙标志 */
            break;
        }		
       
		
		case NRFX_UARTE_EVT_ERROR:                              /* 硬件错误 */
        {                         
            timer1_clear();
            break;
        
        }
			
		
		case NRFX_UARTE_EVT_RX_DONE:                            // 缓冲满回调
        {
            m_rx_len   = p_event->data.rxtx.bytes;
            m_rx_ready = true;
    
            NRF_UARTE0->TASKS_STOPRX = 1;                         // 立即停，溢出字节进不来 
            (void)NRF_UARTE0->TASKS_STOPRX;
           

                
            memset(char_global_buf,'\0',sizeof(char_global_buf));                     // 清除char_global_buf，以备后面保存有效数据
            memcpy(char_global_buf,uart_dma_rx_data(),m_rx_len);  // 将接收到的有效数据copy到char_global_buf 中
            
            timer1_stop_and_clear();        // 定时器停止+ 清零
            uart_dma_rx_start();             //  从启以备下次继续接收
            break;
        
        }		
		 default: 
			break;                                         /* RX_DONE 等其他事件忽略 */
    }
}

/*******************************************************************
********************************************************************
* 名称：
* 功能： timer1 事件回调，定时时间到，执行此函数
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
*  * 工作流程：
 *   数据到达 → RXDRDY → PPI CH0  清零定时器
 *                     → PPI CH1 启动定时器
 *                    
 *
 *   帧结束 → TIMER1 超时 → 本函数被调用
 *          → 读取计数器获取帧长度
 *          → 终止当前 DMA 传输
 *          → 通知主循环处理数据
*********************************************************************
*********************************************************************/
static void timer_evt_handler(nrf_timer_event_t event_type, void * p_context)
{
	uint16_t amount;                                          // 已收到的有效数据的字节数 
    uint32_t timeout = 0;                    // 超时计数器
    (void)p_context;                                          // 不使用上下文 
    if (event_type != NRF_TIMER_EVENT_COMPARE0)               // 只处理 CC0 比较事件 
    {
		return;
	} 
//	led_toggle(LED_17);                                   // 调试用语句
	timer1_stop();                                        // 定时器停止计数
	timer1_clear();                                       // 清零定时器的计数
	
	// 先停止，在读取amount，此时 AMOUNT 不再更新，这点很重要
	NRF_UARTE0->TASKS_STOPRX = 1;          // 停止DMA接收
//	while(NRF_UARTE0->EVENTS_ENDRX == 0);  // 等待停止完成
    while(NRF_UARTE0->EVENTS_ENDRX == 0)
    {
        if(++timeout > 100000)              // 超时退出，防止 ISR 死锁
        {
            NRF_UARTE0->EVENTS_ENDRX = 0;
            timer1_clear();
            return;
        }
    }
    
	NRF_UARTE0->EVENTS_ENDRX = 0;          // 清零操作
	
//	(void)(volatile uint32_t)NRF_UARTE0->RXD.AMOUNT;
    amount = (uint16_t)NRF_UARTE0->RXD.AMOUNT;  // 读取内存中保存的有效数据长度
	
    if (amount > 0)  //  接收到有效数据的个数大于0时
    {
//        led_toggle(LED_19);                                 // 调试用语句
//		nrfx_uarte_rx_abort(&m_uarte);   /* 终止当前 DMA（此时 AMOUNT 不再更新） */
		NRF_UARTE0->TASKS_STOPRX = 1;                         // 立即停，溢出字节进不来 
		(void)NRF_UARTE0->TASKS_STOPRX;
        nrfx_uarte_rx_abort(&m_uarte);                        // 清 nrfx 内部 ram 状态 /

		memset(char_global_buf,'\0',UART_RX_BUF_SIZE);                   // 清除char_global_buf，以备后面保存有效数据
		memcpy(char_global_buf,uart_dma_rx_data(),amount);  // 将接收到的有效数据copy到char_global_buf 中
        m_rx_len   = amount;                                // 将有效数据的长度赋值给全局变量 m_rx_len
        m_rx_ready = true;                                  // 将接收到有效数据的标志位置位
    }
    else  //  没有收到数据
    {
//        led_toggle(LED_20);                                 // 调试用语句
        timer1_clear();                                       // 清零定时器的计数
    }

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：  串口回调函数
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void  uarte_dma_init(void)
{
    nrfx_err_t err;

	ppi_channel_disable(0);  //先强制关闭所有可能残留的 PPI 通道
	ppi_channel_disable(1);
//	ppi_channel_disable(2);  //  新增
	
	/* ENABLE=0 强制硬件干净态 */
//    NRF_UARTE0->ENABLE = 0;
	
	nrfx_uarte_uninit(&m_uarte);          // 初始化前先停止，防止残留状态导致错误
	nrf_gpio_cfg_default(TX_PIN_NUMBER);  // 第1步：引脚硬件复位，清空过往GPIO配置
    nrf_gpio_cfg_default(RX_PIN_NUMBER);  // 第2步：引脚硬件复位，清空过往GPIO配置
	nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);//  RX 引脚必须手动配置为带上拉输入
    // 填充 UARTE 配置结构体
    nrfx_uarte_config_t uart_cfg =
    {
        .pseltxd    = TX_PIN_NUMBER,                    // 配置TX引脚
        .pselrxd    = RX_PIN_NUMBER,                    // 配置RX引脚
        .pselcts    = NRF_UARTE_PSEL_DISCONNECTED,      // 禁用CTS流控
        .pselrts    = NRF_UARTE_PSEL_DISCONNECTED,      // 禁用RTS流控
		.p_context  = NULL,                             //  用户自定义参数指针
        .hwfc       = NRF_UARTE_HWFC_DISABLED,          // 关闭硬件流控
        .parity     = NRF_UARTE_PARITY_EXCLUDED,        // 无奇偶校验
        .baudrate   = UART_DMA_BAUDRATE,                // 配置串口波特率
        .interrupt_priority = 4                         // 直接赋值中断优先级(0~3)，SDK15.2标准取值
    };

    // 初始化UARTE驱动，SDK15.2 固定3个入参
    err = nrfx_uarte_init(&m_uarte, &uart_cfg, uarte_evt_handler);
    APP_ERROR_CHECK(err);

	nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);           //  RX 引脚必须手动配置为带上拉输入
	
	nrf_uarte_int_disable(NRF_UARTE0, NRF_UARTE_INT_RXTO_MASK);       //  RXTO 中断关闭
//	nrf_uarte_int_disable(NRF_UARTE0, NRF_UARTE_INT_ENDRX_MASK);      // endrx 中断关闭
	
	timer1_init(UART_RX_TIMEOUT_US, 6,timer_evt_handler);             // 初始化 TIMER1, 10 ms 帧超时
	
	


	
	// ppi配置 CH0: RXDRDY → TIMER1 CLEAR
	// 串口0 每收到一个字节数据，就联动触发time1的清零动作（也就是说，只要有字符接收到，就一直在清零timer1，不会超时）
	ppi_channel_setup(
                      0,   /* 使用 PPI 通道 0 */
                      nrfx_uarte_event_address_get(&m_uarte, NRF_UARTE_EVENT_RXDRDY),
                      timer1_task_addr_get(NRF_TIMER_TASK_CLEAR));
					  
	ppi_channel_setup(1,
                     nrfx_uarte_event_address_get(&m_uarte, NRF_UARTE_EVENT_RXDRDY),
                     timer1_task_addr_get(NRF_TIMER_TASK_START));
					  
	// =====新增PPI CH2：RXSTARTED 启动Timer，上电/重启RX自动开启计时====
//	ppi_channel_setup(2,
//					  nrfx_uarte_event_address_get(&m_uarte, NRF_UARTE_EVENT_RXSTARTED),
//					  timer1_task_addr_get(NRF_TIMER_TASK_START));
	
//	ppi_channel_fork_setup(0,timer1_task_addr_get(NRF_TIMER_TASK_START));  // 收到一个字节数据后打开定时器
    
    
	
	// ppi 配置，CH1: RXSTARTED → TIMER1 START
	// 开始接收字符时间发生时，就联动触发timer1 开始计时
//	ppi_channel_setup(
//                       1,   /* 使用 PPI 通道 1 */
//                       nrfx_uarte_event_address_get(&m_uarte, NRF_UARTE_EVENT_RXSTARTED),
//                       timer1_task_addr_get(NRF_TIMER_TASK_START));
	
	/* 使能两个 PPI 通道 */
    ppi_channel_enable(0);   //  使能 CH0
    ppi_channel_enable(1);   //  使能 CH1
//	ppi_channel_enable(2);

	
	// 启动首次接收
	timer1_clear();          // TIMER1 计数器归零
	uart_dma_rx_start();     // 启动 UARTE 接收
	

	

}

/*******************************************************************
********************************************************************
* 名称：
* 功能： DMA 发送
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
uart_dma_err_t uart_dma_send(uint8_t const * p_data, uint16_t length)  /* DMA 发送 */
{
    nrfx_err_t err;
    if (p_data == NULL || length == 0)    /* 参数无效 */
	{ 
		return UART_DMA_ERR_INVALID_PARAM; 
	}
    err = nrfx_uarte_tx(&m_uarte, p_data, length);             /* 调用 nrfx_uarte_tx */
    if (err == NRFX_SUCCESS)                     /* 成功，置忙标志 */
	{ 
		m_tx_busy = true; 
		return UART_DMA_OK; 
	} 
    else if (err == NRFX_ERROR_BUSY)  /* 发送忙 */
	{ 
		return UART_DMA_ERR_BUSY; 
	}      
    else  /* 其他错误 */
	{ 
		return UART_DMA_ERR_INVALID_PARAM; 
	}                         
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：查询 TX 是否忙碌,
* 参数：
* 返回值：
m_tx_busy = true= 忙
m_tx_busy = false = 闲
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool uart_dma_is_tx_busy(void)  
{ 
	return m_tx_busy; 
}        
/*******************************************************************
********************************************************************
* 名称：
* 功能：有新帧？ 
* 参数：
* 返回值：
m_rx_ready = true = 有新数据
m_rx_ready = false = 无新数据
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool   uart_dma_rx_ready(void)  
{ 
	return m_rx_ready; 
}       
/*******************************************************************
********************************************************************
* 名称：
* 功能：数据指针（内部缓冲），返回接收数据的地址
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
uint8_t * uart_dma_rx_data(void)
{ 
	return m_rx_buf;  
}      

/*******************************************************************
********************************************************************
* 名称：
* 功能：返回接收到的数据长度
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
size_t uart_dma_rx_len(void)    
{ 
	return m_rx_len;  
}       

/*******************************************************************
********************************************************************
* 名称：
* 功能：确认数据读取完毕后，标志置假， 定时器比较通道清除，从启dma接受， 定时器停止并清零
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void uart_dma_rx_done(void)                                         // 确认读毕，重启接收 
{
	
    m_rx_ready = false;                                             //清除标志 
    nrf_timer_event_clear(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0);    // 清除 COMPARE0 挂起事件 
	timer1_clear();                                                 //   复位定时器（PPI 启动）
	
	memset(m_rx_buf,'\0',UART_RX_BUF_SIZE);                         // 清除接收数据的
	uart_dma_rx_start();                                            // 重新启动 DMA 接收
	
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：接收串口数据，并打印接收到的数据，并根据命令提取对应的RSSI数据
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void uart_data_processing (void)
{
    uint8_t command_set_buf[UART_RX_BUF_SIZE + 7] = {0};  // 用于保存和上位机交互的命令
    
    long chang_num = 0 ;                                  // 保存命令设置中的数值
    if(uart_dma_rx_ready() == true)                       // 串口收到数据
	{
        size_t    len    = uart_dma_rx_len();             // 取已收字节数 
//        print("len= %d\r\n",len);                         //  打印收到的字节数

        if( ( len > 0 ) && ( len < UART_RX_BUF_SIZE ) )   // 收到的数据要大于0，小于缓冲buffer的最大值
        {
//            print("%s\r\n",char_global_buf);              // 串口输出串口收到的数据  （回显）
            if(strstr((const char *)char_global_buf,"rssi_set=") != 0)  // 【设置rssi  截取收到的字符串中的字符“RSSI_SET=”】
            {
                memset(command_set_buf,'\0',sizeof(command_set_buf));                                       //  清0 保存截取字符串的数组
                str_extract((const char *)char_global_buf, "rssi_set=","\r\n",(char*)command_set_buf);      //  从整包数据中截取RSSI_SET= 开头， 到\r\n结束之间字符串      
                chang_num = num_str_to_num((char*)command_set_buf,&rssi_change_result,10);                  //  将截取的字符串转换为数字
                // rssi的转换结果为真 +  rssi的阈值范围   -105  <=   rssi   <=  0 才会执行
                if( (rssi_change_result == true ) && (chang_num <= 0)  && (chang_num >= -105) )             //  转换为数字成功标志， true为转换成功，否则失败
                {
//                    rssi_change_result = false ;             //  将转换成功的标志置假
                    rssi_buf = chang_num ;  // 保存设置的rssi值
                    print("[ rssi = %ld , rssi_set_ok ]\r\n",chang_num);  //  打印转换ok的纯数字 
                }
                else // 转换失败，提示
                {
                    print("< rssi is not a plain numeric value，or rssi set error < -105 <= rssi <= 0 >\r\n");
                }  
            }
            
            else if(strstr((const char *)char_global_buf,"feature_value_set=") != 0)  // 【设置特征值】
            {
                memset(command_set_buf,'\0',sizeof(command_set_buf));                                  //  清0 保存截取字符串的数组
                str_extract((const char *)char_global_buf, "feature_value_set=","\r\n",(char*)command_set_buf); // 从整包数据中截取feature_value_set= 开头， 到\r\n结束之间字符串      
                memset(feature_raw,'\0',sizeof(feature_raw));  
                feature_set((const char *)command_set_buf);  //  将截取的 十六进制的字符串转换为真正的十六进制数，保存在feature_raw 中
                if( feature_global_flag == true)
                {
                    for(char i = 0 ; i < feature_raw_len; i++)
                    {
                        if(i == 0 )
                        {
                            print("[ feature_value = 0x");
                        }
                        print("%02x",feature_raw[i]) ;
                    }
                    print(" ]\r\n");
                   
                }
                else
                {
                     print("< the number of hexadecimal characters must be even >\r\n");
                }
            }
            
            else if(strstr((const char *)char_global_buf,"start_scanning\r\n") != 0)  // 【设置开始扫描，截取收到的字符串中的字符“start_advertising\r\n”】
            {
                run_flag_b = false ;           //  通过start_scanning 方式启动的时候，将默认配置启动的标志位置假， 互斥
                run_flag_a = false;  
                start_advertising_flag = true;    //  允许广播  
                print("[ start_scanning_ok ]\r\n");
            }
            
            else if(strstr((const char *)char_global_buf,"byte_order_set\r\n") != 0)  // 设置mac的大小端显示字节序
            {
                byte_order_flag = true ;   // 字节序逆序输出标志置位
                print("[ byte_order_set_ok ]\r\n");
            }
            
            else if(strstr((const char *)char_global_buf,"use_default_settings\r\n") != 0)  // 设置默认rssi= -60 开始自动扫描
            {
                start_advertising_flag = true;        //  允许广播 
                run_flag_a = true  ;                  //  运行扫描广播的标志置为真，开始扫描
                run_flag_b = true ;                   // 使用默认配置标准置真，在ble_app.c 文件里以此为判断条件
                if(rssi_change_result == false)       //  如果没有主动设置rssi时， 使用默认rssi =  RSSI_DEFAULT_VALUE (否则使用设置的rssi阈值)
                {
                    rssi_buf = RSSI_DEFAULT_VALUE;    //  默认的rssi阈值
                }
                rssi_change_result = true;            //  rssi设置结果   true 设置成功  (在 ble_app.c 里打印广播的判断是：rssi_change_result 为真+满足rssi阈值
                mask_raw_data_global_flag = true ;    // 扫描原始数据输出
                print("[ default_settings_ok ]\r\n");
            }
            
            else if(strstr((const char *)char_global_buf,"mask_raw_data\r\n") != 0)  // 设置mac的大小端显示字节序
            {
                mask_raw_data_global_flag = true ;   // 扫描原始数据输出
                print("[ mask_raw_data_ok ]\r\n");
            }
            else if(strstr((const char *)char_global_buf,"decoding_setting\r\n") != 0)  // 设置ble 数据的解密方式
            {
                decoding_set_global_flag = !decoding_set_global_flag ;   // 解码数据设置成功 （true = 明文解析数据， false = 异或解密方式解密数据，默认false）
                if(decoding_set_global_flag == true)
                {
                    print("[ plaintext_set_ok ]\r\n");  // 明文解析数据
                }
                else
                {
                    print("[ cipnertext_set_ok ]\r\n");  // 异或解密数据
                }
                
            }
            
            
            else    //  查找字符串失败提示
            {
                print("< command is error >\r\n"); 
            }
        }
        else        // 接收的字符串数量 = 缓冲buffer大小的时候提示，因为有可能有错误
        {
            print("< possible error >\r\n");
        }

        uart_dma_rx_done();                      // 确认处理完毕，重新启动接收 (将uart_dma_rx_ready 标志置假)
		
	}

}





