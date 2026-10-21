#include "button.h"
#include "stm32f4xx.h"
#include "systick_delay_freertos.h"
#include "buzzer.h"
#include "stdbool.h"
#include "my_task.h"
#include "lcd.h"
#include "usart_dma_all.h"
#include "variable_global.h"
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"
#include "queue.h"                             // 使用消息队列要包含的头文件
#include "semphr.h"                            //信号量头文件包含
#include "my_task.h"
#include "power_ctrl.h"
#include "w25q32.h"
#include "data_processing.h"
#include "string.h"
//======================================================================================
bool cmd_set_flag_from_buttom = false ;  //  用于控制按键key3按下后，执行使用从flash中读取的数据（rssi阈值， 特征值，cmd 唯一码值）接收数据
bool cmd_select_flag = false ;     //  cmd_select_flag 在key2 按键按下后翻转， 在key3按下后，根据此来选择执行时按照 新的cmd 唯一码执行，还是按照旧的cmd唯一码执行
bool decoding_setting_flag = false ;   //  在中断里面按一次取反一次， false = 通知nrf52832 按照异或方式解析数据， true = 按照明文方式接续数据
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* PB15 :KEY1  (L:按下   H :没有按下)
  PB14: KEY2  (L:按下   H :没有按下)
  PC7  :KEY3  (L:按下   H :没有按下)
*********************************************************************
*********************************************************************/
void button_pin_init(void)
{
	//初始化GPIO 的结构体变量
	GPIO_InitTypeDef  GPIO_TypeDef_init;
	//外部中断线的结构体变量，用来初始化外部中断
	EXTI_InitTypeDef  exti_init_type_strcut;
	//初始化中断优先级的结构体变量
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOE时钟
	
	//初始化 pin的输入模式，速度，pin选择
	// PB15
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_15;
	GPIO_TypeDef_init.GPIO_Mode = GPIO_Mode_IN;       //普通输入模式
//	GPIO_TypeDef_init.GPIO_OType = GPIO_OType_PP;     //推挽输出
	GPIO_TypeDef_init.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	GPIO_TypeDef_init.GPIO_PuPd = GPIO_PuPd_UP;       //上拉 （低有效配置为上拉）
	GPIO_Init(GPIOB,&GPIO_TypeDef_init);
	// PB14
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_TypeDef_init);
	// PC7
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOC,&GPIO_TypeDef_init);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//注意要打开SYSCFG时钟 使能外设中断时钟
	//外部中断线的映射关系 PB14的中断
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource14);//PB14 连接到中断线14
	//外部中断线的映射关系 PB15的中断
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource15);//PB15 连接到中断线15
	//外部中断线的映射关系 PC7的中断
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource7);//PC7 连接到中断线7
	
	
	//初始化外部中断 PB15
	exti_init_type_strcut.EXTI_Line = EXTI_Line15;                 //选定 15线
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;         //选择中断模式
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Falling;     //下降沿触发中断
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;                   //使能线中断
	EXTI_Init(&exti_init_type_strcut);
	
	
	//初始化外部中断 PB14
	exti_init_type_strcut.EXTI_Line = EXTI_Line14;                 //选定 14线
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;         //选择中断模式
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Falling;     //下降沿触发中断
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;                   //使能线中断
	EXTI_Init(&exti_init_type_strcut);
	
	//初始化外部中断PC7
	exti_init_type_strcut.EXTI_Line = EXTI_Line7;
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init_type_strcut);
	
	
	//配置中断分组 PB15
	//配置中断分组 PB14
	nvic_init_type_strcut.NVIC_IRQChannel = EXTI15_10_IRQn;
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 7;
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic_init_type_strcut);
	
	
	
	//配置中断分组 PC7
	nvic_init_type_strcut.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 9;
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic_init_type_strcut);

	
}
// PC7  :KEY3  (L:按下   H :没有按下)
void key3_button_pin_init(void)
{
	//初始化GPIO 的结构体变量
	GPIO_InitTypeDef  GPIO_TypeDef_init;
	//外部中断线的结构体变量，用来初始化外部中断
	EXTI_InitTypeDef  exti_init_type_strcut;
	//初始化中断优先级的结构体变量
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOE时钟
	
	//初始化 pin的输入模式，速度，pin选择
	// PC7
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_7;
	GPIO_TypeDef_init.GPIO_Mode = GPIO_Mode_IN;       //普通输入模式
//	GPIO_TypeDef_init.GPIO_OType = GPIO_OType_PP;     //推挽输出
	GPIO_TypeDef_init.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	GPIO_TypeDef_init.GPIO_PuPd = GPIO_PuPd_UP;       //上拉 （低有效配置为上拉）
	GPIO_Init(GPIOC,&GPIO_TypeDef_init);
	
	
	
	
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);//注意要打开SYSCFG时钟 使能外设中断时钟
	
	//外部中断线的映射关系 PC7的中断
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource7);//PC7 连接到中断线7
	
	
	//初始化外部中断PC7
	exti_init_type_strcut.EXTI_Line = EXTI_Line7;
	exti_init_type_strcut.EXTI_Mode = EXTI_Mode_Interrupt;
	exti_init_type_strcut.EXTI_Trigger = EXTI_Trigger_Falling;
	exti_init_type_strcut.EXTI_LineCmd = ENABLE;
	EXTI_Init(&exti_init_type_strcut);
	
	
	//配置中断分组 PC7
	nvic_init_type_strcut.NVIC_IRQChannel = EXTI9_5_IRQn;
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 9;
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&nvic_init_type_strcut);

	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* KEY2  PB14  按下后将cmd_select_flag 取反，然后读取旧 cmd 唯一码解析值 或者 清零 旧的 cmd 唯一码解析值
  KEY1  PB15  按下后，触发usart2 发送“decodeing_seeting\r\n”,从而触发nrf52832设置解析方式
*********************************************************************
*********************************************************************/
void  EXTI15_10_IRQHandler(void)
{
    unsigned char lcd_display_buf[31] = {0};
//	BaseType_t    pxHigherPriorityTaskWoken = pdFALSE ;																						
	uint32_t  ulReturn;                            // 保存临界代码段保护函数的返回值
	
	systick_delay_ms(20);

	
	ulReturn = taskENTER_CRITICAL_FROM_ISR();        // 进入临界代码段
    if(usart2_cmd_set_flag == true)  // 上电初始化后， 此标志为真，表示允许设置，一旦开始扫描后，此标志为fasle，设置失效
    {
        if(EXTI_GetITStatus(EXTI_Line14) != RESET)       // 监测是那个中断线产生的中断  key2  pb14
        {
            cmd_select_flag = ! cmd_select_flag;
            if(cmd_select_flag == true)
            {
                memset(SPI1_FLASH_BUF,'\0',4096);
                memset(lcd_display_buf,'\0',31);
                w25q32_flash_read(SPI1_FLASH_BUF,0x00000049,3,2);  // 唯一码读取 (要读取3个字节，目的是把\R\N读取出来)
                if (str_extract_before_first((const char *)SPI1_FLASH_BUF, "\r\n", (char*)lcd_display_buf, 3)  == true)  // 找到了有效字符
                {
                    memset(SPI1_FLASH_BUF,'\0',4096);
                    hex_arr_to_str(lcd_display_buf, 1, (char*)SPI1_FLASH_BUF); // 将十六进制数据，转换为十六进制字符串，保存在SPI1_FLASH_BUF
                    display_ascii_string(121,261,8,(u8*)"0X",DarkGreen,Black); 
                    display_ascii_string(137,261,8,(u8*)SPI1_FLASH_BUF,DarkGreen,Black);  
                    usart1_printf("OLD_ONLY_LEN = 0X%s\r\n",SPI1_FLASH_BUF);
                }
                else
                { 
                    display_ascii_string(5,414,6,(u8*)"OLD_ONLY_LEN SET ERROR",Red,Black); 
                    usart1_printf("OLD_ONLY_LEN SET ERROR\r\n");
                }
            }
            else
            {
                lcd_fill_color(121,261,153,277,Black);  // 清理显示区域
            }
        }
        //  处理 key1按键按下时，给nrf52832 发送指令，设置解析数据的方式
        else if(EXTI_GetITStatus(EXTI_Line15) != RESET)  //  key1 pb15 按键按下
        {
            usart2_printf("decoding_setting\r\n"); // 给nrf52832 发送执行
//            usart1_printf("decoding_setting\r\n"); // 给nrf52832 发送执行
            decoding_setting_flag = ! decoding_setting_flag ; //  取反解析方式标志 （false = 异或结束数据，  true = 明文解析数据） 
        }
    }
	
	
//	if(pxHigherPriorityTaskWoken == pdTRUE)
//	{
//		/*如果函数xTaskResumeFromISR()返回值为pdTRUE，那么说明要恢复的这个
//		任务的任务优先级等于或者高于正在运行的任务(被中断打断的任务),所以在
//		退出中断的时候一定要进行上下文切换！*/
//		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
//	}
	EXTI_ClearITPendingBit(EXTI_Line15);      // 清除中断标志
	EXTI_ClearITPendingBit(EXTI_Line14);      // 清除中断标志
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                                       // 退出临界代码段保护

}


/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* key3   pc7
*********************************************************************
*********************************************************************/
void  EXTI9_5_IRQHandler(void)
{
	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
//	BaseType_t    xsemaphore_back ; //保存二值信号量give 函数的返回值
//	BaseType_t    pxHigherPriorityTaskWoken = pdFALSE ;
	//**************************************************************************************
	ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
	systick_delay_ms(20);
	
	if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	{
		cmd_set_flag_from_buttom  = true ;     //  true设置结束，并开始进入扫描工作阶段
        lcd_fill_color(85,357,317,373,Black);  //  cmd set mode  字符清除

	}
	
	
//	// 判断是否在出中断的时候要进行任务切换
//	if(pxHigherPriorityTaskWoken == pdTRUE)
//	{
//		portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
//	}
	EXTI_ClearITPendingBit(EXTI_Line7);//清除中断标志
	taskEXIT_CRITICAL_FROM_ISR( ulReturn );  // 退出临界代码段保护

}




