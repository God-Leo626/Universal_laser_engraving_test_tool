/* 按键通道：ISR记录下降沿 -> 任务等待20ms确认低电平 -> 执行原设置动作。
 * 所有LCD、Flash、串口操作在串口2业务任务执行，避免中断阻塞与共享外设重入。 */
#include "button.h"
#include "my_task.h"
#include "lcd.h"
#include "usart_dma_all.h"
#include "w25q32.h"
#include "data_processing.h"
#include "FreeRTOS.h"
#include "task.h"

bool cmd_set_flag_from_buttom = false; /* KEY3使用保存的配置开始扫描。 */
bool cmd_select_flag = false; /* KEY2启用/停用旧CMD规则。 */
bool decoding_setting_flag = false; /* KEY1切换密文/明文解析。 */
static volatile bool key1_edge = false, key2_edge = false, key3_edge = false;
static bool key1_ready = false, key2_ready = false, key3_ready = false;

void button_pin_init(void)
{
	//初始化GPIO 的结构体变量
	GPIO_InitTypeDef  GPIO_TypeDef_init;
	//外部中断线的结构体变量，用来初始化外部中断
	EXTI_InitTypeDef  exti_init_type_strcut;
	//初始化中断优先级的结构体变量
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	GPIO_StructInit(&GPIO_TypeDef_init);
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
	
	GPIO_StructInit(&GPIO_TypeDef_init);
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

void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line14) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line14);
        key2_edge = true;
    }
    if (EXTI_GetITStatus(EXTI_Line15) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line15);
        key1_edge = true;
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line7);
        key3_edge = true;
    }
}

static void button_toggle_old_rule(void)
{
    u8 stored[3];
    char hex_text[3];
    cmd_select_flag = !cmd_select_flag;
    if (!cmd_select_flag)
    {
        lcd_fill_color(121, 261, 153, 277, Black);
        return;
    }
    w25q32_flash_read(stored, 0x49, sizeof(stored), 2);
    /* 保存格式为原始字节+CRLF；不得当作C字符串解析（值0也可能存在）。 */
    if (w25q32_last_operation_ok() && stored[1] == '\r' && stored[2] == '\n')
    {
        hex_arr_to_str(stored, 1, hex_text);
        display_ascii_string(121, 261, 8, (u8 *)"0X", DarkGreen, Black);
        display_ascii_string(137, 261, 8, (u8 *)hex_text, DarkGreen, Black);
        usart1_printf("OLD_ONLY_LEN = 0X%s\r\n", hex_text);
    }
    else
    {
        display_ascii_string(5, 414, 6, (u8 *)"OLD_ONLY_LEN SET ERROR", Red, Black);
        usart1_printf("OLD_ONLY_LEN SET ERROR\r\n");
    }
}

/* 每键独立状态：0等待边沿，1确认按下，2等待稳定释放；长按只触发一次。 */
static bool button_debounce(u8 index, bool edge, bool pressed, TickType_t now)
{
    static u8 state[3] = {0};
    static TickType_t since[3] = {0};
    const TickType_t delay = pdMS_TO_TICKS(20) + 1; /* 补一tick，消抖不短于20ms。 */
    if (state[index] == 0)
    {
        if (edge) { state[index] = 1; since[index] = now; }
    }
    else if (state[index] == 1)
    {
        if (!pressed) state[index] = 0;
        else if ((TickType_t)(now - since[index]) >= delay)
        {
            state[index] = 2;
            since[index] = now;
            return true;
        }
    }
    else
    {
        if (pressed) since[index] = now;
        else if ((TickType_t)(now - since[index]) >= delay) state[index] = 0;
    }
    return false;
}

void button_sample_events(void)
{
    bool edge1, edge2, edge3, pressed1, pressed2, pressed3;
    TickType_t now = xTaskGetTickCount();
    /* 此临界区只交换3个布尔量，串口中断延迟保持极短。 */
    taskENTER_CRITICAL();
    edge1 = key1_edge; key1_edge = false;
    edge2 = key2_edge; key2_edge = false;
    edge3 = key3_edge; key3_edge = false;
    taskEXIT_CRITICAL();
    pressed1 = button_debounce(0, edge1, KEY1 == 0, now);
    pressed2 = button_debounce(1, edge2, KEY2 == 0, now);
    pressed3 = button_debounce(2, edge3, KEY3 == 0, now);
    key1_ready = key1_ready || pressed1;
    key2_ready = key2_ready || pressed2;
    key3_ready = key3_ready || pressed3;
}

void button_process_events(void)
{
    bool pressed1, pressed2, pressed3;
    button_sample_events();
    pressed1 = key1_ready; key1_ready = false;
    pressed2 = key2_ready; key2_ready = false;
    pressed3 = key3_ready; key3_ready = false;
    if (usart2_cmd_set_flag)
    {
        if (pressed2) button_toggle_old_rule();
        if (pressed1)
        {
            usart2_printf("decoding_setting\r\n");
            decoding_setting_flag = !decoding_setting_flag;
        }
    }
    if (pressed3)
    {
        cmd_set_flag_from_buttom = true;
        lcd_fill_color(85, 357, 317, 373, Black);
    }
}
