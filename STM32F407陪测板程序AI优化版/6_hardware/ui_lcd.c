/* 启动入口：配置板载硬件、画固定界面、显示保存配置、读取NRF固件版本。
 * 此函数在启动调度器前运行；之后LCD由串口2业务任务独占。 */
#include "ui_lcd.h"
#include "systick_delay_freertos.h"
#include "usart_dma_all.h"
#include "buzzer.h"
#include "lcd.h"
#include "coolkit_logo.h"
#include "power_ctrl.h"
#include "spi1.h"
#include "button.h"
#include "variable_global.h"
#include "w25q32.h"
#include "time3.h"
#include "uart_select.h"
#include "data_processing.h"
#include <string.h>
#include <stdio.h>

#define STM_FIRMWARE_VERSION "V 1_0_4"

static void ui_draw_layout(void)
{
    static const char *labels[] = {"MAC     :", "SN      :", "CH      :", "RSSI    :",
        "RSSI THR:", "FEATURE :", "MASK    :", "DEFAULT :", "START   :", "ONLY LEN:"};
    unsigned int i;
    char text[100];
    lcd_clear(Black);
    lcd_draw_line(1, 53, 319, 53, Blue);
    lcd_draw_line(1, 54, 319, 54, Blue);
    display_ascii_string(40, 55, 12, (u8 *)"BLE PASSIVE SCANNING", Silver, Black);
    lcd_draw_rectangle(1, 79, 319, 279, DimGray);
    for (i = 0; i < sizeof(labels) / sizeof(labels[0]); ++i)
        display_ascii_string(5, (u16)(81 + i * 20), 8, (u8 *)labels[i], Silver, Black);
    lcd_fill_color(2, 283, 318, 349, DarkGray);
    lcd_draw_rectangle(1, 353, 319, 438, DimGray);
    display_ascii_string(5, 357, 8, (u8 *)"LOG     :", Silver, Black);
    lcd_draw_line(1, 428, 319, 428, DimGray);
    display_ascii_string(85, 357, 8, (u8 *)"cmd set mode", Indigo, Black);
    snprintf(text, sizeof(text), "STM VERSION : %s", STM_FIRMWARE_VERSION);
    display_ascii_string(3, 439, 6, (u8 *)text, Silver, Black);
    snprintf(text, sizeof(text), "COMPILE TIME: %s,%s", __DATE__, __TIME__);
    display_ascii_string(3, 465, 6, (u8 *)text, Silver, Black);
    lcd_draw_rectangle(209, 438, 319, 478, DimGray);
    lcd_draw_rectangle(1, 438, 319, 478, DimGray);
    usart1_printf("STM VERSION : %s\r\n", STM_FIRMWARE_VERSION);
    usart1_printf("COMPILE TIME: %s,%s\r\n", __DATE__, __TIME__);
}

/* 小缓冲读取文本项并补NUL；Flash格式仍使用原地址及CRLF结尾。 */
static bool ui_read_text(u32 address, u16 count, char *out, u32 out_size)
{
    u8 raw[32] = {0};
    if (count >= sizeof(raw)) return false;
    w25q32_flash_read(raw, address, count, 2);
    return w25q32_last_operation_ok() &&
           str_extract_before_first((const char *)raw, "\r\n", out, out_size);
}

static void ui_display_only_length(u32 address, u16 x, const char *name)
{
    u8 raw[3];
    char text[3];
    w25q32_flash_read(raw, address, sizeof(raw), 2);
    if (w25q32_last_operation_ok() && raw[1] == '\r' && raw[2] == '\n')
    {
        hex_arr_to_str(raw, 1, text);
        display_ascii_string(x, 261, 8, (u8 *)"0X", Indigo, Black);
        display_ascii_string(x + 16, 261, 8, (u8 *)text, Indigo, Black);
        usart1_printf("%s = 0X%s\r\n", name, text);
    }
    else
    {
        usart1_printf("%s SET ERROR\r\n", name);
        if (address == 0x46)
            display_ascii_string(85, 261, 8, (u8 *)"NEW_ONLY_LEN SET ERROR", Red, Black);
        else display_ascii_string(5, 414, 6, (u8 *)"OLD_ONLY_LEN SET ERROR", Red, Black);
    }
}

static void ui_display_saved_settings(void)
{
    u8 rssi;
    char text[31];
    unsigned int width;
    w25q32_flash_read(&rssi, 0, 1, 2);
    if (w25q32_last_operation_ok())
    {
        usart1_printf("DEFAULT RSSI    = %d\r\n", (int)(s8)rssi);
        if ((s8)rssi <= 0 && (s8)rssi >= -105)
        {
            snprintf(text, sizeof(text), "%d", (int)(s8)rssi);
            display_ascii_string(85, 161, 8, (u8 *)text, Indigo, Black);
        }
    }
    else
    {
        usart1_printf("FLASH READ ERROR\r\n");
        display_ascii_string(85, 161, 8, (u8 *)"FLASH READ ERROR", Red, Black);
    }
    if (ui_read_text(0x09, 31, text, sizeof(text)))
    {
        usart1_printf("DEFAULT FEATURE = %s\r\n", text);
        display_ascii_string(85, 181, 8, (u8 *)text, Indigo, Black);
    }
    if (ui_read_text(0x28, 30, text, sizeof(text)))
    {
        width = (unsigned int)strlen(text) * 12;
        /* 过长项目名从左侧显示，避免无符号减法回绕到屏幕外。 */
        display_ascii_string((u16)(width < 320 ? (320 - width) / 2 : 0),
                             14, 12, (u8 *)text, Indigo, Black);
    }
    else display_ascii_string(112, 14, 12, (u8 *)"PRJ_NAME", Silver, Black);
    ui_display_only_length(0x46, 85, "NEW_ONLY_LEN");
    ui_display_only_length(0x49, 121, "OLD_ONLY_LEN");
    display_ascii_string(165, 261, 8, (u8 *)"CIPHERTEXT", Indigo, Black);
}

static void ui_print_command_help(void)
{
    usart1_printf("[ rssi_set_cmd    ] = 7F 07 81 00 00 C4 42\r\n");
    usart1_printf("[ freture_cmd     ] = 7F 0C 82 00 00 02 01 05 1B FF CA A6\r\n");
    usart1_printf("[ start_scanning  ] = start_scanning\r\n");
    usart1_printf("[ mask_raw_set    ] = mask_raw_data\r\n");
    usart1_printf("[ prj_name_set    ] = prj_set=S60TPF\r\n");
    usart1_printf("[ use_default_set ] = use_default_settings\r\n");
    usart1_printf("[ only_len_set    ] = only_len_set=06\r\n");
}

static void ui_read_nrf_version(void)
{
    char received[1024] = {0};
    char version[14], text[100];
    unsigned int i, count = 0, copied;
    for (i = 0; i < 40; ++i)
    {
        systick_delay_ms(50); /* 上电启动信息累计等待最多2秒。 */
        USART2_REVEIVE_OK = false;
        copied = read_ringbuffer_count_for_usart1(&usart2_receive_buffer,
                     received + count, sizeof(received) - 1 - count);
        count += copied;
        received[count] = '\0';
        if (str_intercept(received, "[ version :  ", ", time", version, sizeof(version)))
        {
            usart1_printf("[ NRF VERSION : %s ]\r\n", version);
            snprintf(text, sizeof(text), "NRF VERSION : %s", version);
            display_ascii_string(3, 452, 6, (u8 *)text, Silver, Black);
            return;
        }
    }
    usart1_printf("%s\r\n", received);
    usart1_printf("NRF52832 VERSION IS UNKNOW \r\n");
    display_ascii_string(3, 452, 6, (u8 *)"NRF52832 VERSION IS UNKNOW", Red, Black);
}

void ui_display_hardware_init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    systick_delay_freerots_init(168);
    lcd_init();
    power_ctrl_init();
    uart_select_init();
    uart_select(true);
    buzzer_init();
    spi1_init();
    lcd_show_image(94, 196, coolkit_logo_133x87);
    systick_delay_ms(600);
    lcd_clear(Black);
    usart1_init(115200);
    usart2_init(115200);
    /* APB1定时器84MHz / 8400 / 5000 = 2Hz，启动时仅配置不运行。 */
    time3_init(5000 - 1, 8400 - 1);
    button_pin_init();
    power_ctrl_nrf52832(true);
    ui_draw_layout();
    ui_display_saved_settings();
    ui_print_command_help();
    ui_read_nrf_version();
    buzzer_ctrl(10, true);
}
