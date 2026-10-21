#include "sdk_config.h"
#include "nrfx_uarte.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "led.h"
#include "buzzer.h"
#include "button.h"
#include "touch_pad.h"
//#include "nrf_52832_uart.h"
//#include "uart_dma.h"
#include "nrf_uart.h"
//#include "app_uart.h" 
#include "my_printf.h"
#include "timer.h"
#include "uarte_52832_dma.h"
#include "nrfx_clock.h"
#include "ble_app.h"
#include "data_processing.h"
//=======================================================================================




int main (void)
{	
//	nrfx_clock_hfclk_start();                        // 启动外部 32 MHz 晶振
//	while (!nrfx_clock_hfclk_is_running()) {};

	ble_passive_scanning_init ();                         //SoftDevice 必须在一切外设之前初始化
	uarte_dma_init();
    led_init();                                         // 初始化led引脚
    
	unsigned int i = 0 ;                                // 用于led亮灭的计数

    print("[ uarte_dma_init is ok !                                  ]\r\n");
    print("[ bluetooth low energy protocol stack init ok             ]\r\n");
    
    print("[ system init ok                                          ]\r\n");
    print("[ command list: < Must reset the system to configure    > ]\r\n");
    print("[ command list: < Commands must end with \\r\\n         > ]\r\n");
    print("1 ---> [ rssi_set=-xx\\r\\n ,xx=decimal_number            ] --- < set rssi command           >\r\n");
    print("2 ---> [ feature_value_set=020105\\r\\n,020105=hex_number ] --- < set feature_value command  >\r\n");
    print("3 ---> [ mask_raw_data\\r\\n                              ] --- < mask_raw_data command      >\r\n");
    print("4 ---> [ byte_order_set\\r\\n                             ] --- < byte_order_set command     >\r\n");
    print("5 ---> [ use_default_settings\\r\\n                       ] --- < default_setting command    >\r\n");  
    print("6 ---> [ start_scanning\\r\\n                             ] --- < set start_scanning command >\r\n");
    print("7 ---> [ decoding_setting\\r\\n                           ] --- < set decoding mode          >\r\n");
    
    print("[ version :  V 1_0_7, time : %s %s ]\r\n",__DATE__,__TIME__); // 格式不能随便修改，stm32407要凭借格式解析版本

	while(1) 
	{
        if(start_advertising_flag == true )  // 是否允许扫描的开关，
        {
            main_loop_process();             // 处理协议中接收到的广播数据并打印
        }
		else
        {
            while(g_adv_rd != g_adv_wr) // 命令模式下静默排空缓冲区，避免历史脏数据堵塞
            {
                g_adv_rd = (g_adv_rd + 1) % ADV_EVT_BUF_SIZE;
            }
        
            uart_data_processing();          // 接收串口命令，设置指令等  （此函消耗时间比较长。在上电初始化时，led闪烁较慢）
        }
        
        if( i++ == 50000)
        {
            i = 0 ;
            led_toggle(LED_04);    
        }
	}
}


