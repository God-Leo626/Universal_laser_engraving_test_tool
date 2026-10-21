#include "ui_lcd.h"
#include "stm32f4xx.h"
#include "systick_delay_freertos.h"
#include "usart_dma_all.h"
#include "dma.h"
#include "buzzer.h"
#include "led.h"
#include "fixture_start.h"
#include "lcd.h"
#include "coolkit_logo.h"
#include "power_ctrl.h"
#include "spi1.h"
#include "spi2.h"
#include "button.h"
#include <string.h>        // 使用字符串处理函数
#include <stdio.h>         //使用sprintf函数时包含
#include "variable_global.h"
#include "stm32_crc.h"
#include  "stm32f4xx_chip_id.h"
#include "w25q32.h"
#include "time3.h"

#include "calibration.h"


#include "relay.h"
#include "my_task.h"

#include "stdlib.h"
#include "uart_select.h"
#include "data_processing.h"
//*****************************************************************************************************************





unsigned char work_num_buf[10] = {0};   //  保存设置的工单号的前6个字符
//*****************************************************************************************************************
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-12-24
* 举例：
* 备注：1
* 
* 
*********************************************************************
*********************************************************************/
void ui_display_hardware_init(void)
{
	char nrf52832_software_version_buf[1024] = {0};    // 保存nrf52832的固件版本
    char nrf52832_version_buf[14] = {0} ;              //  保存截取好的nrf52832的固件版本
	unsigned char lcd_display_buf[31] = {0};
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4
	systick_delay_freerots_init(168);              //初始化延时函数

	
	lcd_init();                                    // lcd初始化
	power_ctrl_init() ;                            // nrf52832的电源控制引脚初始化
    uart_select_init();                            //  串口选择为引脚初始化 uart1
    uart_select(1);                                //  选择连接uart1
	buzzer_init();                                 // 蜂鸣器初始化
	

	spi1_init();                                   // spi1 接口初始化()

	lcd_show_image(94 ,196,coolkit_logo_133x87);    // 开机显示logo
	systick_delay_ms(600);

	lcd_clear(Black);                               // 将整个屏幕初始化为晒黑色
	
	
	
	usart1_init(115200);                            // 串口1初始化
    
    
    usart2_init(115200);                  // 串口2 初始化，串口2 和 nrf52832对接， 需要115200的波特率
    
    
   



	time3_init(5000-1,8400-1);             // Tout= ((arr+1)*(psc+1))/Tclk；500ms一次中断
	                                  // 定时器时钟84M，分频系数8400，所以84M/8400=10Khz的计数频率，计数5000次为500ms  
//    time3_run( true)  ;   
//	TIM5_Int_Init(5000-1,8400-1);			   // 一次0.1ms， 5000次就是 500ms
							   
	//*****************************************************************************************************************

	
	button_pin_init();                 // 初始化按键

    power_ctrl_nrf52832(1);                         // 打开nrf52832的电源 

	//=============================UI===============================================
    
	lcd_clear(Black);                               // 将整个屏幕初始化为晒黑色
//    display_ascii_string(112,1,12,(u8*)"PRJ_NAME",Silver,Black); 
    lcd_draw_line(1, 53, 320, 53,Blue) ;          // 标题横线
    lcd_draw_line(1, 54, 320, 54,Blue) ;          // 标题横线
    
    display_ascii_string(40,55,12,(u8*)"BLE PASSIVE SCANNING",Silver,Black); 
    
    lcd_draw_rectangle(1,79,319,279,DimGray);  // 显示矩形 (从mac开始到frature结束框起来)

    
    display_ascii_string(5,81,8,(u8*) "MAC     :",Silver,Black); 
    display_ascii_string(5,101,8,(u8*)"SN      :",Silver,Black); 
    display_ascii_string(5,121,8,(u8*)"CH      :",Silver,Black); 
    display_ascii_string(5,141,8,(u8*)"RSSI    :",Silver,Black); 
   
    display_ascii_string(5,161,8,(u8*)"RSSI THR:",Silver,Black); 
    display_ascii_string(5,181,8,(u8*)"FEATURE :",Silver,Black);
    
    display_ascii_string(5,201,8,(u8*)"MASK    :",Silver,Black);
    display_ascii_string(5,221,8,(u8*)"DEFAULT :",Silver,Black);
    display_ascii_string(5,241,8,(u8*)"START   :",Silver,Black);
    display_ascii_string(5,261,8,(u8*)"ONLY LEN:",Silver,Black);
    
    
    
    lcd_fill_color(2,283,318,349,DarkGray);  // 显示 色块矩形
    
    lcd_draw_rectangle(1,353,319,438,DimGray);  // 显示log 矩形
     
    display_ascii_string(5,357,8,(u8*)"LOG     :",Silver,Black); //  显示log字符
    lcd_draw_line(1, 428, 319, 428,DimGray) ;          // 显示进度栏上面的横线
    
    display_ascii_string(85,357,8,(u8*)"cmd set mode",Indigo,Black); //  显示命令设置模式
    
    
    //   ===============显示固件版本信息====================================
    memset((void*)string_buf,0,sizeof((const char*)string_buf)); // 清空string_buf
    sprintf((char *)string_buf,"STM VERSION : %s","V 1_0_6");     // 打印 stm 固件版本
    display_ascii_string(3,439,6,(u8*)string_buf,Silver,Black); 
    
//    memset((void*)string_buf,0,sizeof((const char*)string_buf)); // 清空string_buf
//    sprintf((char *)string_buf,"NRF VERSION : %s","V 1_0_2");    // 打印 nrf的固件版本
//    display_ascii_string(3,453,6,(u8*)string_buf,Silver,Black);
//    
    memset((void*)string_buf,0,sizeof((const char*)string_buf)); // 清空string_buf
    sprintf((char *)string_buf,"COMPILE TIME: %s,%s",__DATE__,__TIME__);  // stm软件编译时间
    display_ascii_string(3,465,6,(u8*)string_buf,Silver,Black);
    
    //   ===============显示定时器时间显示框====================================
    lcd_draw_rectangle(209,438,319,478,DimGray);  // 显示时间显示矩形 
    //   ===============显示版本框====================================
    lcd_draw_rectangle(1,438,319,478,DimGray);  // 显示版本框矩形 
	
    usart1_printf("STM VERSION : V 1_0_6\r\n");
//    usart1_printf("NRF VERSION : V 1_0_3\r\n");
	usart1_printf("COMPILE TIME: %s,%s\r\n",__DATE__,__TIME__);

	
	
    //******************从flash中读取  【1 rssi】  ***************************************************
	memset(SPI1_FLASH_BUF,'\0',4096);
    w25q32_flash_read(SPI1_FLASH_BUF,0x00000000,1,2);  // rssi阈值读取 （0----  -105）
    usart1_printf("DEFAULT RSSI    = %d\r\n",(int8_t)SPI1_FLASH_BUF[0]);
    if( ((int8_t)SPI1_FLASH_BUF[0] <= 0) && ((int8_t)SPI1_FLASH_BUF[0] >= -105)) // 将读取到的数据转换为由符号数并判断
    {
        memset(lcd_display_buf,'\0',31);
        sprintf((char *)lcd_display_buf,"%d" ,(int8_t) SPI1_FLASH_BUF[0]  );    // 把 SPI1_FLASH_BUF[0]  当初有符号数，按照十进制打印成字符保存在lcd_display_buf
        display_ascii_string(85,161,8,(u8*)lcd_display_buf,Indigo,Black);       // 在led 上显示rssi阈值
    }
    
    
     //******************从flash中读取【 2 feature】   *****************************************************
    memset(SPI1_FLASH_BUF,'\0',4096);
    w25q32_flash_read(SPI1_FLASH_BUF,0x00000009,31,2); // feature 特征值读取
    usart1_printf("DEFAULT FEATURE = %s\r\n",SPI1_FLASH_BUF);
    memset(lcd_display_buf,'\0',31);
    if (str_extract_before_first((const char *)SPI1_FLASH_BUF, "\r\n", (char*)lcd_display_buf, 31)  == true)  // 找到了有效字符
    {
        display_ascii_string(85,181,8,(u8*)lcd_display_buf,Indigo,Black);
    }
    
    
    //******************从flash中读取  【3 项目名称】 *****************************************************
    memset(SPI1_FLASH_BUF,'\0',4096);
    w25q32_flash_read(SPI1_FLASH_BUF,0x00000028,30,2); // 项目名称读取
    memset(lcd_display_buf,'\0',31);
    if (str_extract_before_first((const char *)SPI1_FLASH_BUF, "\r\n", (char*)lcd_display_buf, 31)  == true)  // 找到了有效字符
    {
        display_ascii_string( (320 - strlen((const char*)lcd_display_buf) * 12 ) / 2,10,12,(u8*) lcd_display_buf,Indigo,Black);    // 居中显示prj name
    } 
    else
    {
        display_ascii_string(112,10,12,(u8*)"PRJ_NAME",Silver,Black); //  显示默认的项目名称 
    }
    
    //******************从flash中读取 【4 only len  唯一码 ---cmd [ 新 ]的解析规则 cmd < 0x83】  ***************************************************
    memset(SPI1_FLASH_BUF,'\0',4096);
    memset(lcd_display_buf,'\0',31);
    w25q32_flash_read(SPI1_FLASH_BUF,0x00000046,3,2);  // 唯一码读取 (要读取3个字节，目的是把\R\N读取出来)
    if (str_extract_before_first((const char *)SPI1_FLASH_BUF, "\r\n", (char*)lcd_display_buf, 3)  == true)  // 找到了有效字符
    {
        memset(SPI1_FLASH_BUF,'\0',4096);
        hex_arr_to_str(lcd_display_buf, 1, (char*)SPI1_FLASH_BUF); // 将十六进制数据，转换为十六进制字符串，保存在SPI1_FLASH_BUF
        display_ascii_string(85,261,8,(u8*)"0X",Indigo,Black); 
        display_ascii_string(101,261,8,(u8*)SPI1_FLASH_BUF,Indigo,Black);  
        usart1_printf("NEW_ONLY_LEN = 0X%s\r\n",SPI1_FLASH_BUF);
    }
    else
    { 
        display_ascii_string(85,261,8,(u8*)"NEW_ONLY_LEN SET ERROR",Red,Black); 
        usart1_printf("NEW_ONLY_LEN SET ERROR\r\n");
    }
    
    //******************从flash中读取 【5 only len  唯一码 ---cmd [ 旧 ] 的解析规则 cmd >= 0x83】  ***************************************************
    memset(SPI1_FLASH_BUF,'\0',4096);
    memset(lcd_display_buf,'\0',31);
    w25q32_flash_read(SPI1_FLASH_BUF,0x00000049,3,2);  // 唯一码读取 (要读取3个字节，目的是把\R\N读取出来)
//    usart1_printf(">>>%02x,%02x,%02x\r\n",SPI1_FLASH_BUF[0],SPI1_FLASH_BUF[1],SPI1_FLASH_BUF[2]);
    if (str_extract_before_first((const char *)SPI1_FLASH_BUF, "\r\n", (char*)lcd_display_buf, 3)  == true)  // 找到了有效字符
    {
        memset(SPI1_FLASH_BUF,'\0',4096);
        hex_arr_to_str(lcd_display_buf, 1, (char*)SPI1_FLASH_BUF); // 将十六进制数据，转换为十六进制字符串，保存在SPI1_FLASH_BUF
        display_ascii_string(121,261,8,(u8*)"0X",Indigo,Black); 
        display_ascii_string(137,261,8,(u8*)SPI1_FLASH_BUF,Indigo,Black);  
        usart1_printf("OLD_ONLY_LEN = 0X%s\r\n",SPI1_FLASH_BUF);
    }
    else
    { 
        display_ascii_string(5,414,6,(u8*)"OLD_ONLY_LEN SET ERROR",Red,Black); 
        usart1_printf("OLD_ONLY_LEN SET ERROR\r\n");
    }
    
    // =================================从flash 中读取 工单号信息=====================================
    // =============================================================================================
    memset(SPI1_FLASH_BUF,'\0',4096);
    memset(lcd_display_buf,'\0',31);
    w25q32_flash_read(SPI1_FLASH_BUF,0x00000052,10,2);  // 唯一码读取 (要读取10个字节，目的是把\R\N读取出来)
    if (str_extract_before_first((const char *)SPI1_FLASH_BUF, "\r\n", (char*)lcd_display_buf, 10)  == true)  // 找到了有效字符
    {
        lcd_fill_color(5,36,315,48,Black);  //刷新以前显示
        display_ascii_string(136 ,36,6,(u8*)lcd_display_buf,Indigo,Black);    // 居中显示 工单号
        work_num_select_global_flag = true ;   // 从flash 中读取到有效的工单号后，对接收到的sn启动判断
        memset(work_num_buf,'\0',10); //  将保存工单号的字符数组清零
        memcpy(work_num_buf,lcd_display_buf,6); // 将工单号的前6个字符保存进  work_num_buf
    }

    //  上电默认显示xor 异或校验方式解析数据， lcd上显示提示信息
    display_ascii_string(165,261,8,(u8*)"CIPHERTEXT",Indigo,Black); // 密文解析数据
    //******************【5 打印命令信息】  ***************************************************
    
    
    
    usart1_printf("[ rssi_set_cmd    ] = 7F 07 81 00 00 C4 42\r\n");
    usart1_printf("[ freture_cmd     ] = 7F 0C 82 00 00 02 01 05 1B FF CA A6\r\n");
    usart1_printf("[ start_scanning  ] = start_scanning\r\n");
//    usart1_printf("[ byte_order_set  ] = byte_order_set\r\n");
    usart1_printf("[ mask_raw_set    ] = mask_raw_data\r\n");
    usart1_printf("[ prj_name_set    ] = prj_set=S60TPF\r\n");
    usart1_printf("[ use_default_set ] = use_default_settings\r\n");
    usart1_printf("[ only_len_set    ] = only_len_set=06\r\n");
    usart1_printf("[ work_num_set=   ] = work_num_set=253043\r\n");
    usart1_printf("[ no_work_num\\r\\n ] = no_work_num\\r\\n");
    
    
    //******************【6 延时， 释放cpu，切换其他任务运行】  ***************************************************
    
    
    
    //******************【7 处理自动获取nrf52832模块固件版本号的代码】  ***************************************************
    unsigned char i = 0;   // 用于for 循环
    unsigned int accum_len = 0;  //  用于记录从nrf52832的串口发送过来的数据
    for ( i = 0 ; i < 40;i++)
    {
        systick_delay_ms(50);  //  延时等待stm32的串口接收 nrf52832的数据
        if(USART2_REVEIVE_OK == true) // stm32 串口2 接收到了数据   
        {
            USART2_REVEIVE_OK = false ;
            unsigned int str_len = 0 ;
            NVIC_DisableIRQ(USART2_IRQn);   // 只关USART2中断，防止IDLE中断在读取途中写环形缓冲
            str_len =   get_ringbuffer_length_for_usart1(&usart2_receive_buffer) ;// 读取环形缓冲区的已经接收到的数据长度
            if(str_len > sizeof(nrf52832_software_version_buf) - 1 - accum_len)
            {
                str_len = sizeof(nrf52832_software_version_buf) - 1 -  accum_len;  // 限幅防溢出
            }
            read_ringbuffer_for_usart1(&usart2_receive_buffer,(char*)&nrf52832_software_version_buf[accum_len],str_len); 
           
            NVIC_EnableIRQ(USART2_IRQn);    // 立即恢复
            accum_len += str_len;
            nrf52832_software_version_buf[accum_len] = '\0';   // 累积后补结束符
            // 截取到了nrf52832的固件版本 （nrf52832 上电串口打印输出， stm32的串口2 收到数据并截取）
            if(str_intercept((const char *)nrf52832_software_version_buf, "[ version :  ", ", time",nrf52832_version_buf, 14) == true )
            {
                usart1_printf("[ NRF VERSION : %s ]\r\n",nrf52832_version_buf);
                memset((void*)string_buf,0,sizeof((const char*)string_buf)); // 清空string_buf
                sprintf((char *)string_buf,"NRF VERSION : %s",nrf52832_version_buf);    // 打印 nrf的固件版本
                display_ascii_string(3,452,6,(u8*)string_buf,Silver,Black);
                break ; // 退出for 循环
            }
        }   
    }
    if(i >= 40)
    {
        usart1_printf("%s\r\n",nrf52832_software_version_buf);
        usart1_printf("NRF52832 VERSION IS UNKNOW \r\n");
        display_ascii_string(3,452,6,(u8*)"NRF52832 VERSION IS UNKNOW",Red,Black);
    }
    
    
    
    //******************【8  蜂鸣器鸣叫 10ms】  ***************************************************    
    buzzer_ctrl(10,true);  // 蜂鸣器名叫10ms
    
    
}
