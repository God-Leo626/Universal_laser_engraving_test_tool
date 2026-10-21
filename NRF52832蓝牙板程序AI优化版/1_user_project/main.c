/**
 * 程序入口：初始化 -> 串口配置 -> 广播过滤与输出。
 * SoftDevice 必须先于应用外设初始化。扫描硬件在初始化时就已启动，
 * 但配置阶段丢弃广播；只有收到开始命令后才处理广播，重新配置需要复位。
 */
#include <stdint.h>

#include "ble_app.h"
#include "app_config.h"
#include "led.h"
#include "my_printf.h"
#include "uarte_52832_dma.h"
#include "uart_commands.h"

/* 保留原来的循环计数心跳；这是运行指示，不是固定周期的计时器。 */
#define LED_HEARTBEAT_LOOP_COUNT 50000u

static void print_startup_info(void)
{
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

    /* STM32F407 按此格式解析版本，保留版本字符串和字段格式。 */
    print("[ version :  V 1_0_7, time : %s %s ]\r\n", __DATE__, __TIME__);
}

int main(void)
{
    uint32_t heartbeat_count = 0;

    ble_passive_scanning_init();
    uarte_dma_init();
    led_init();
    print_startup_info();

    for (;;)
    {
        if (g_app_config.scanning_started)
        {
            main_loop_process();          /* 每次至多处理一个广播报告。 */
        }
        else
        {
            ble_discard_pending_reports();
        }

        /* 两个阶段都维护接收；命令层在运行阶段只丢弃数据。 */
        uart_data_processing();

        if (heartbeat_count++ == LED_HEARTBEAT_LOOP_COUNT)
        {
            heartbeat_count = 0;
            led_toggle(LED_04);
        }
    }
}
