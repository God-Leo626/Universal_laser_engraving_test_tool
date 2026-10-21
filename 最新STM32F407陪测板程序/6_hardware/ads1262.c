#include "ads1262.h"
#include "stm32f4xx.h"
#include "spi2.h"
#include "systick_delay_freertos.h"
#include "stdio.h"
#include "stdbool.h"
#include "lcd.h"
#include "usart_dma_all.h"
#include "string.h"
#include "power_ctrl.h"
#include "stdlib.h"
#include "button.h"
#include "variable_global.h"
#include "w25q32.h"
#include "relay.h"
#include "time3.h"
#include "max49918.h"
#include "arithmetic.h"
#include "my_task.h"
//===========================================================================
u8  ch1_ofcal_global_calibration_buf[3] = {0};  // 用于保存CH1通道的系统偏移校准值 3 个 u8数据  [0]保存07寄存器数据[1]保存08寄存器数据 [2]保存09寄存器数据
u8  ch2_ofcal_global_calibration_buf[3] = {0};  // 用于保存CH1通道的系统偏移校准值 3 个 u8数据  [0]保存07寄存器数据[1]保存08寄存器数据 [2]保存09寄存器数据
u8  ch3_ofcal_global_calibration_buf[3] = {0};  // 用于保存CH1通道的系统偏移校准值 3 个 u8数据  [0]保存07寄存器数据[1]保存08寄存器数据 [2]保存09寄存器数据
u8  ch4_ofcal_global_calibration_buf[3] = {0};  // 用于保存CH1通道的系统偏移校准值 3 个 u8数据  [0]保存07寄存器数据[1]保存08寄存器数据 [2]保存09寄存器数据

unsigned char string_voltage[100] = {0};    // 用于sprintf函数打印字符串
u8 threshold_ch1_global_flag = 0 ;          // 用于统计ch1 的达到阈值电流值的次数
u8 threshold_ch2_global_flag = 0 ;          // 用于统计ch2 的达到阈值电流值的次数
u8 threshold_ch3_global_flag = 0 ;          // 用于统计ch3 的达到阈值电流值的次数
u8 threshold_ch4_global_flag = 0 ;          // 用于统计ch4 的达到阈值电流值的次数

bool ch1_test_circle_global_flag = true ;   // ch1测试是否结束的标准 （true: 继续循环测试 false：结束本次测试）
bool ch2_test_circle_global_flag = true ;   // ch2测试是否结束的标准 （true: 继续循环测试 false：结束本次测试）
bool ch3_test_circle_global_flag = true ;   // ch3测试是否结束的标准 （true: 继续循环测试 false：结束本次测试）
bool ch4_test_circle_global_flag = true ;   // ch4测试是否结束的标准 （true: 继续循环测试 false：结束本次测试）

bool continuous_test_mode_global_flag = false ; //  连续测试模式标志  （默认不是连续模式， false ： 不连续测试， true ：连续测试）
u8   channel_4_test_over_global_flag = 0x00 ;   // 此标志只要用于记录4个通道的测试是否完毕（不论成功与否），4个通道全部测试完毕过后，通过此标志关闭电源



bool ch1_auto_range_select_global_flag = false ;  // ch1 自动量程选择标识，没有超量程为真，初始化为假
bool ch2_auto_range_select_global_flag = false ;  // ch2 自动量程选择标识，没有超量程为真，初始化为假
bool ch3_auto_range_select_global_flag = false ;  // ch3 自动量程选择标识，没有超量程为真，初始化为假
bool ch4_auto_range_select_global_flag = false ;  // ch4 自动量程选择标识，没有超量程为真，初始化为假
// bit0 =1 : ch1 测试通过
// bit1 =1 : ch2 测试通过
// bit2 =1 : ch3 测试通过
// bit3 =1 : ch4 测试通过

double function_l_global_threshold_value = 0;  // 用于保存atof函数中转换出来的 l 阈值
double function_h_global_threshold_value = 0;  // 用于保存atof函数中转换出来的 h 阈值

//  量程最终选择变量，用于保存测试时选择的量程数据
u8 ch1_test_range_value = 1 ; // 通道1保存量程选择时所确定的增益数据
u8 ch2_test_range_value = 1 ; // 通道2保存量程选择时所确定的增益数据
u8 ch3_test_range_value = 1 ; // 通道3保存量程选择时所确定的增益数据
u8 ch4_test_range_value = 1 ; // 通道4保存量程选择时所确定的增益数据
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
 // PE0   RESET
 // PB12  CS  CS-仅确定串行接口，不控制RST和START信号
 // PE1   ready
 // PB9   START

*********************************************************************
*********************************************************************/
static void ads1262_ctrl_pin_init(void)
{
	GPIO_InitTypeDef GPIO_InitTypeDef_pin;
	// 打开GPIOB  GPIOE 的时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOE, ENABLE);//使能GPIOB GPIOE时钟
	
	// PE0  reset  

	GPIO_InitTypeDef_pin.GPIO_Pin = GPIO_Pin_0 ;      // 选择的PIN脚 
	GPIO_InitTypeDef_pin.GPIO_Mode = GPIO_Mode_OUT ;  //普通输出模式
	GPIO_InitTypeDef_pin.GPIO_OType =  GPIO_OType_PP; //推挽输出
	GPIO_InitTypeDef_pin.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_pin);
	
	// PE1 ready
	GPIO_InitTypeDef_pin.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitTypeDef_pin.GPIO_Mode = GPIO_Mode_IN;       //普通输入模式
//	GPIO_InitTypeDef_pin.GPIO_OType = GPIO_OType_PP;     //推挽输出
	GPIO_InitTypeDef_pin.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	GPIO_InitTypeDef_pin.GPIO_PuPd = GPIO_PuPd_UP;       //上拉 （低有效配置为上拉）
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_pin);
	
	// PB12  cs
//	GPIO_InitTypeDef_pin.GPIO_Pin = GPIO_Pin_12 ;      // 选择的PIN脚 
//	GPIO_InitTypeDef_pin.GPIO_Mode = GPIO_Mode_OUT ;   // 普通输出模式
//	GPIO_InitTypeDef_pin.GPIO_OType =  GPIO_OType_PP;  // 推挽输出
//	GPIO_InitTypeDef_pin.GPIO_Speed = GPIO_Speed_100MHz;
//	GPIO_Init(GPIOB,&GPIO_InitTypeDef_pin);
	
	// PB9  start
	GPIO_InitTypeDef_pin.GPIO_Pin = GPIO_Pin_9 ;      // 选择的PIN脚 
	GPIO_InitTypeDef_pin.GPIO_Mode = GPIO_Mode_OUT ;  //普通输出模式
	GPIO_InitTypeDef_pin.GPIO_OType =  GPIO_OType_PP; //推挽输出
	GPIO_InitTypeDef_pin.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_pin);
	
//	GPIO_SetBits(GPIOB,GPIO_Pin_12);    // cs 拉高，不片选
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);  // cs 拉低，片选
	GPIO_SetBits(GPIOB,GPIO_Pin_9);    // 拉高，不开始转换
	
	

}
/*******************************************************************
********************************************************************
* 名称：void ads1262_read_register(u8 start_address,u8 num_reg,u8* p_data)
* 功能：读寄存器数据
* 参数：start_address：起始地址   
		num_reg ：寄存器个数  
		p_data：读到的寄存器数据起始地址
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_read_register(u8 start_address,u8 num_reg,u8* p_data)
{
	u8 i=0;                                              // 用于循环计数
//	ADS1262_CS = 0;                                      // 片选
	spi2_read_write_byte(CMD_READ_REG + start_address);  // 先发送ads1262的读取命令
	spi2_read_write_byte(num_reg - 1);                   // 在发送读取寄存器的个数
	for(i=0;i<num_reg;i++)
	{
		p_data[i] = spi2_read_write_byte(0x00);
	}
//	ADS1262_CS = 1;                                     // 不片选
}
/*******************************************************************
********************************************************************
* 名称：void ads1262_write_register(u8 start_address,u8 num_reg,u8* p_data)
* 功能：写数据到ads1262的寄存器
* 参数：start_address：起始地址   
		num_reg ：寄存器个数  
		p_data：要写到寄存器的数据的起始地址
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void ads1262_write_register(u8 start_address,u8 num_reg,u8* p_data)
{
	u8 i=0;                                               // 用于循环计数
//	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_WRITE_REG + start_address);  // 先发送ads1262的写命令
	spi2_read_write_byte(num_reg - 1);                    // 在发送写几个寄存器的数量
	for(i=0;i<num_reg;i++)
	{
		spi2_read_write_byte(p_data[i]);                  // 发送要写的内容
	}
//	ADS1262_CS = 1;                                       // 不片选
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void get_ads1262_id(void)
{
	u8 data = 0;
//	ADS1262_CS = 0;                                       // 片选
	ads1262_read_register(ID, 1,&data);  
	
	if(data & 0x20)// 001
	{
		usart1_printf("ID:ADS1263\r\n");  // 表示1263
	}
	else  // 000
	{
		usart1_printf("ID:ADS1262\r\n");  // 表示1262
	}
//	ADS1262_CS = 1;                                       // 不片选
}

/*******************************************************************
********************************************************************
* 名称：void ads1262_hd_reset(void)
* 功能：复位ads262
* 参数：无
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void ads1262_hd_reset(void)
{
//	ADS1262_CS = 0;                                       // 片选
	ADS1262_RESET = 0 ;
	systick_delay_ms(4);  //拉低的时间不能太长，超过9ms，进入掉电模式，在掉电模式中退出，拉高要等待9ms
	ADS1262_RESET = 1 ;
	systick_delay_ms(50);
//	ADS1262_CS = 1;                                       // 不片选
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_soft_reset(void)
{
//	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_RESET);
//	ADS1262_CS = 1;                                       // 不片选
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_start_cmd_adc1(void)
{
//	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_START_ADC1);
//	ADS1262_CS = 1;                                       // 不片选
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_start_hd_adc1(void)
{
//	ADS1262_CS = 0;                                       // 片选
	ADS1262_START = 0;                                    // 开始转换
//	ADS1262_CS = 1;                                       // 不片选
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_stop_hd_adc1(void)
{
//	ADS1262_CS = 0;                                       // 片选
	ADS1262_START = 1;                                 // 停止转换
//	ADS1262_CS = 1;                                       // 不片选
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_stop_adc1(void)
{
//	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_STOP_ADC1);
//	ADS1262_CS = 1;                                       // 不片选
}

void ads1262_CMD_SFOCAL1(void)
{
//	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_SFOCAL1);
//	ADS1262_CS = 1; 
}

void ads1262_CMD_SYGCAL1(void)
{
//	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_SYGCAL1);
//	ADS1262_CS = 1; 
}

void ads1262_CMD_SYOCAL1(void)
{
//	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_SYOCAL1);
//	ADS1262_CS = 1; 
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void wait_for_ads1262_adc1_conversion_ready(void)
{
//	while (!ADS1262_READY) ; 
	while (ADS1262_READY) ; 	
}








/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void ads1262_register_init(void)
{
	u8 read_form_flash_value_buf[3] = {0} ;                  //  用于保存从flash中读取的上午 07,08,09 寄存器的校准值
	u8 ads1262_reg_read_buf[21] = {0};                       //  ads1262 有21个寄存器 ,保存读出来的寄存器值
	u8 ads1262_reg_write_buf[21] = {0};                      //  ads1262 有21个寄存器 ,保存要写的寄存器值
	u8 sps_flag = 0x00 ;                                     //  保存采样率寄存器值
	ads1262_read_register(ID,21,ads1262_reg_read_buf);       //  保存读取出来的值
	for(u8 i = 0 ;i < 21 ; i++)
	{
		usart1_printf("RESET_register[0X%02X] :%02X\r\n",i,ads1262_reg_read_buf[i]);
	}
	usart1_printf("========================================\r\n");

	
	//改变要写入寄存器的值
	ads1262_reg_write_buf[ID] = ads1262_reg_read_buf[ID] ;// 03
	ads1262_reg_write_buf[POWER] =      0x10;         // 复位1，偏置关闭，内部参考启用 1 开内部  0 关内部参考    10
	ads1262_reg_write_buf[INTERFACE] =  0x0c;         // 关闭串口超时，关闭状态字节，关闭校验（4个字节数据）0x06  6字节  00  4字节  0c
	ads1262_reg_write_buf[MODE0] =      0x10;         // 正的参考电压极性（0：正常 1 ：反向），连续转换模式(0:连续转换 1 ：单次转换)，斩波禁能（00：斩波禁止 01： 斩波使能），转换0延时  0000 0000   0x10    （10）
	                                                  // 单次转换模式下，开始时redy 为1，数据ok后为低
	ads1262_reg_write_buf[MODE1] =      0x60;         // sinC4模式0x60    sinc3:0x40  sinc2:0x20   sinc1:0x00   
	ads1262_reg_write_buf[MODE2] =      0x80 | MODE2_DATA_RATE_60SPS  ;         // PGA使能=1，2.5sps   1000 0000  0x80
	ads1262_reg_write_buf[INPMUX] =     0x89;         // AIN8+  AIN9-
	
//	ads1262_reg_write_buf[OFCAL0] =     0x00;
//	ads1262_reg_write_buf[OFCAL1] =     0x00;
//	ads1262_reg_write_buf[OFCAL2] =     0x00;
	ads1262_reg_write_buf[FSCAL0] =     0x00;
	ads1262_reg_write_buf[FSCAL1] =     0x00;
	ads1262_reg_write_buf[FSCAL2] =     0x40;
	ads1262_reg_write_buf[IDACMUX] =    0xBB; 
	ads1262_reg_write_buf[REFMUX] =     0x09;       // 000: Internal 2.5 V reference - P (default)  09
	                                               // 001 :AIN0  001 : AIN1

	// 把准备好的值写回到ads1262中
	ads1262_write_register(ID,21,&ads1262_reg_write_buf[ID]);
	//==========================================
	w25q32_flash_read((u8*)&read_form_flash_value_buf[0],0x0000003C,3,1);          // 读取CH1 07,08,09 寄存器值  读取3个长度
	ads1262_write_register(OFCAL0,1,&read_form_flash_value_buf[0]);  // 写 07 寄存器
	ads1262_write_register(OFCAL1,1,&read_form_flash_value_buf[1]);  // 写 08 寄存器	
	ads1262_write_register(OFCAL2,1,&read_form_flash_value_buf[2]);  // 写 09 寄存器
	//==============================================

	// 清零 ads1262_reg_read_buf
	memset(ads1262_reg_read_buf,0,strlen((const char*)ads1262_reg_read_buf));
	// 读取寄存器
	ads1262_read_register(ID,21,ads1262_reg_read_buf); //  保存读取出来的值
	
	for(u8 i = 0 ;i < 21 ; i++)
	{
		usart1_printf("SET_register[0X%02X] :%02X\r\n",i,ads1262_reg_read_buf[i]);
	}
	ads1262_start_hd_adc1();    // 拉低start 引脚 （防止后面命令读取的时候干扰）
	
	// 显示采样率
	lcd_fill_color(114,419,204,431,Tan);   // 覆盖上一次显示的字符，为下一次显示准备
	ads1262_read_register(MODE2,1,&sps_flag);       //  保存读取出来的值
	if((sps_flag & 0x0F) == MODE2_DATA_RATE_2_5SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[   2.5 SPS   ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_5SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[    5 SPS    ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_10SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[   10  SPS   ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_16_SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  16.6  SPS  ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_20SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[   20  SPS   ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_50SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[   50  SPS   ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_60SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[   60  SPS   ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_100SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[   100 SPS   ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_400SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[   400 SPS   ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_1200SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  1200  SPS  ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_2400SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  2400  SPS  ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_4800SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  4800  SPS  ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_7200SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  7200  SPS  ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_14400SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  14400 SPS  ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_19200SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  19200 SPS  ]",Black,Tan);
	}
	else if((sps_flag & 0x0F) == MODE2_DATA_RATE_38400SPS)
	{
		display_ascii_string(114,419,6,(u8*)"[  38400 SPS  ]",Black,Tan);
	}
	
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_SFOCAL1_calibration(void)
{
	u8 ads1262_reg_read_buf[21] = {0};                       //  ads1262 有21个寄存器 ,保存读出来的寄存器值
	u8 ads1262_reg_write_buf[21] = {0};                      //  ads1262 有21个寄存器 ,保存要写的寄存器值
	ads1262_read_register(ID,21,ads1262_reg_read_buf); //  保存读取出来的值
	
	//改变要写入寄存器的值
	ads1262_reg_write_buf[ID] = ads1262_reg_read_buf[ID] ;
	ads1262_reg_write_buf[POWER] =      0x10;         // 复位1，偏置关闭，内部参考启用 1 开内部  0 关内部参考
	ads1262_reg_write_buf[INTERFACE] =  0x00;         // 关闭串口超时，关闭状态字节，关闭校验（4个字节数据）0x06  6字节  00  4字节
	ads1262_reg_write_buf[MODE0] =      0x00;         // 正的参考电压极性，连续转换模式，斩波禁能，转换0延时  0000  0000
	                                                  // 单次转换模式下，开始时redy 为1，数据ok后为低
	ads1262_reg_write_buf[MODE1] =      0x60;         // sinC4模式0x60    sinc3： 0x40
	ads1262_reg_write_buf[MODE2] =      0x88;         // PGA使能=1，1200sps   1000 1001
	ads1262_reg_write_buf[INPMUX] =     0xff;         // 输入通道全部断开 1111 1111 

	ads1262_reg_write_buf[REFMUX] =     0x09;        // 000: Internal 2.5 V reference - P (default)

	
	//把准备好的值写回到ads1262中
	ads1262_write_register(ID,21,&ads1262_reg_write_buf[ID]);
	

	ads1262_start_hd_adc1();  // 启动转换
	systick_delay_ms(10);     // 延时
	ads1262_CMD_SFOCAL1();    // 偏移自校准
	systick_delay_ms(100);    // 延时
	ads1262_stop_hd_adc1();  // 停止转换
	
	memset(ads1262_reg_read_buf,0,sizeof((const char*)ads1262_reg_read_buf));
	ads1262_read_register(OFCAL0,3,ads1262_reg_read_buf); //  保存读取出来的值
	for(u8 i = 0 ;i < 3 ; i++)
	{
		usart1_printf("register[0X%02X] :%02X\r\n",i+OFCAL0,ads1262_reg_read_buf[i]);
		
	}
	usart1_printf("********************************************************************************************************************\r\n");

}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_calibration_init(void)
{
	spi2_init();                           // 通信接口spi2 初始化 
	ads1262_ctrl_pin_init();               // 其他控制引脚初始化  片选
	ads1262_hd_reset();                    // 复位
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_init(void)
{
	spi2_init();                           // 通信接口spi2 初始化 
	ads1262_ctrl_pin_init();               // 其他控制引脚初始化  片选
	ads1262_hd_reset();                    // 复位
//	ads1262_SFOCAL1_calibration();         // 偏移自校准
	ads1262_register_init();               // 寄存器初始化  	 
}

/*******************************************************************
********************************************************************
* 名称：void ads1262_input_channel_select(u8 channel_num)
* 功能： 选择输入通道
* 参数：channel_num ： 通道配置
INPUT_CHANNEL_1
INPUT_CHANNEL_2
INPUT_CHANNEL_3
INPUT_CHANNEL_4
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void ads1262_input_channel_select(u8 channel_num)
{
	u8 ads1262_reg_read_buf[3] = {0};                       //  ads1262 有21个寄存器 ,保存读出来的寄存器值
	u8 temp = 0 ;   //  用于保存 通道信息
	u8 read_form_flash_value_buf[3] = {0} ; // 用于保存从flash中读取的上午 07,08,09 寄存器的校准值
	if(channel_num == INPUT_CHANNEL_1)
	{
		temp = INPUT_CHANNEL_1 ;
		ads1262_stop_hd_adc1();  // 停止转换
		systick_delay_us(7);     // 等待停止
		ads1262_write_register(INPMUX,1,&temp);  // AIN8+  AIN9-
	
			
		w25q32_flash_read((u8*)&read_form_flash_value_buf[0],0x0000003C,3,1);          // 读取 07,08,09 寄存器值  读取3个长度

		ads1262_write_register(OFCAL0,1,&read_form_flash_value_buf[0]);  // 写 07 寄存器
		ads1262_write_register(OFCAL1,1,&read_form_flash_value_buf[1]);  // 写 08 寄存器	
		ads1262_write_register(OFCAL2,1,&read_form_flash_value_buf[2]);  // 写 09 寄存器
		
		ads1262_read_register(OFCAL0,3,ads1262_reg_read_buf);       //  保存读取出来的值
		for(u8 i = 0 ;i < 3 ; i++)
		{
			usart1_printf("read_calibration_register[0X%02X] :%02X\r\n",i+OFCAL0,ads1262_reg_read_buf[i]);
		}

		
	}
	else if(channel_num == INPUT_CHANNEL_2)
	{
		temp = INPUT_CHANNEL_2 ;
		ads1262_stop_hd_adc1();  // 停止转换
		systick_delay_us(7);     // 等待停止
		ads1262_write_register(INPMUX,1,&temp);  // AIN6+  AIN7-
		
		w25q32_flash_read((u8*)&read_form_flash_value_buf[0],0x0000003F,3,1);          // 读取 07,08,09 寄存器值  读取3个长度
		
		ads1262_write_register(OFCAL0,1,&read_form_flash_value_buf[0]);  // 写 07 寄存器
		ads1262_write_register(OFCAL1,1,&read_form_flash_value_buf[1]);  // 写 08 寄存器	
		ads1262_write_register(OFCAL2,1,&read_form_flash_value_buf[2]);  // 写 09 寄存器
		
		
		ads1262_read_register(OFCAL0,3,ads1262_reg_read_buf);       //  保存读取出来的值
		for(u8 i = 0 ;i < 3 ; i++)
		{
			usart1_printf("read_calibration_register[0X%02X] :%02X\r\n",i+OFCAL0,ads1262_reg_read_buf[i]);
		}

	
	}
	else if(channel_num== INPUT_CHANNEL_3)
	{
		temp = INPUT_CHANNEL_3 ;
		ads1262_stop_hd_adc1();  // 停止转换
		systick_delay_us(7);     // 等待停止
		ads1262_write_register(INPMUX,1,&temp);  // AIN4+  AIN5-
		
		w25q32_flash_read((u8*)&read_form_flash_value_buf[0],0x00000042,3,1);          // 读取 07,08,09 寄存器值  读取3个长度
		
		ads1262_write_register(OFCAL0,1,&read_form_flash_value_buf[0]);  // 写 07 寄存器
		ads1262_write_register(OFCAL1,1,&read_form_flash_value_buf[1]);  // 写 08 寄存器	
		ads1262_write_register(OFCAL2,1,&read_form_flash_value_buf[2]);  // 写 09 寄存器
		
		
		ads1262_read_register(OFCAL0,3,ads1262_reg_read_buf);            //  保存读取出来的值
		for(u8 i = 0 ;i < 3 ; i++)
		{
			usart1_printf("read_calibration_register[0X%02X] :%02X\r\n",i+OFCAL0,ads1262_reg_read_buf[i]);
		}

	
	}
	else if(channel_num== INPUT_CHANNEL_4)
	{
		temp = INPUT_CHANNEL_4 ;
		ads1262_stop_hd_adc1();  // 停止转换
		systick_delay_us(7);     // 等待停止
		ads1262_write_register(INPMUX,1,&temp);  // AIN6+  AIN7-
		
		w25q32_flash_read((u8*)&read_form_flash_value_buf[0],0x00000045,3,1);          // 读取 07,08,09 寄存器值  读取3个长度
		
		ads1262_write_register(OFCAL0,1,&read_form_flash_value_buf[0]);  // 写 07 寄存器
		ads1262_write_register(OFCAL1,1,&read_form_flash_value_buf[1]);  // 写 08 寄存器	
		ads1262_write_register(OFCAL2,1,&read_form_flash_value_buf[2]);  // 写 09 寄存器
		
		ads1262_read_register(OFCAL0,3,ads1262_reg_read_buf);       //  保存读取出来的值
		for(u8 i = 0 ;i < 3 ; i++)
		{
			usart1_printf("read_calibration_register[0X%02X] :%02X\r\n",i+OFCAL0,ads1262_reg_read_buf[i]);
		}

	
	}

}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void ads1262_voltage_swap(int32_t* a, int32_t* b)
{
	int32_t temp = 0;
	temp = *a ;
	*a = *b ;
	*b = temp ;	
}

/*******************************************************************
********************************************************************
* 名称：int32_t ads1262_get_average(int32_t * data, u8 data_leng)
* 功能：将一组数按照从小到的顺序排列后取中间值
* 参数：data ： 要排列的数字指针，data_leng：这组数的长度
* 返回值： 去掉最大最小后的总值的平均数
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static int32_t ads1262_get_average(int32_t * data, u8 data_leng)
{
	int64_t  value_sum = 0 ;
	u8 i = 0 ,j = 0;
	int32_t average = 0 ;
	for ( i = 0 ; i < data_leng ; i++)
	{
		for(j = i+1;j < data_leng;j++)
		{
			if(data[i] > data[j]) //如果前面的数大，交换两个数的位置（从小到大排列）
			{
				ads1262_voltage_swap(&data[i],&data[j]); 
			}
		}
	}
//	for(u8 k = 0 ;k < AVERAGE_DATA_LENG;k++)
//	{
//		usart1_printf("[%d] = [%08X]\r\n",k,data[k]);
//	}
	for(u8 k = 2; k < data_leng - 2; k++)  // 去掉前2个和后2个数在求和
	{
		value_sum += data[k] ;  // 去掉最大，和最小后求和
	}
	
	average = value_sum / (int64_t)(data_leng - 4.0 ) ;
//	usart1_printf("value = [%08X]\r\n",average);
	return average ;
}

/*******************************************************************
********************************************************************
* 名称：void ads1262_SYOCAL1_calibration(u8 channel_select)
* 功能：根据指定通道进行系统偏移校准并将校准值保存在flash中
* 参数：channel_select ：要校准的通道
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
1 ：设置adc连续转换
2：增益和参考电压
3：校准通道选择
4：选择校准类型
    自校准
    系统校准（短路要校准的输入通道）
5：开始转换（启动转换）
6：发送校准命令（SYOCAL1）
7：等待延时（根据采样率和滤波选择延时时间）
8：校准完成
9：读取校准OFCAL[2:0]
10：保存打印
* 
*********************************************************************
*********************************************************************/
void ads1262_SYOCAL1_calibration(u8 channel_select)
{
	u8 m = 0;                                                // 用于指示百分比
	u8 ads1262_reg_read_buf[21] = {0};                       //  ads1262 有21个寄存器 ,保存读出来的寄存器值
	int32_t syocal_1_cali_data_buf[21] = {0};
	int32_t syocal_1_data = 0 ;                             // 保存系统偏移校准的3个字节数据
	u8 ads1262_reg_write_buf[21] = {0};                      //  ads1262 有21个寄存器 ,保存要写的寄存器值
	ads1262_read_register(ID,21,ads1262_reg_read_buf);       //  保存读取出来的值
//	for(u8 i = 0 ;i < 21 ; i++)
//	{
//		usart1_printf("RESET_register[0X%02X] :%02X\r\n",i,ads1262_reg_read_buf[i]);
//	}
//	usart1_printf("***********************************************\r\n");
	
	//改变要写入寄存器的值
	ads1262_reg_write_buf[ID] = ads1262_reg_read_buf[ID] ;
	ads1262_reg_write_buf[POWER] =      0x11;         // 复位1，偏置关闭，内部参考启用 1 开内部  0 关内部参考  11
	ads1262_reg_write_buf[INTERFACE] =  0x0C;         // 关闭串口超时，关闭状态字节，关闭校验（4个字节数据）0x06  6字节  00  4字节
	ads1262_reg_write_buf[MODE0] =      0x00;         // 正的参考电压极性，连续转换模式，斩波禁能，转换0延时  0000  0000
	                                                  // 单次转换模式下，开始时redy 为1，数据ok后为低
	ads1262_reg_write_buf[MODE1] =      0x60;         // sinC4模式0x60    sinc3： 0x40
	ads1262_reg_write_buf[MODE2] =      0x48;         // PGA使能=0，16  ,400sps   0100 1000
	if(channel_select == 1)
	{
		ads1262_reg_write_buf[INPMUX] =     0x89;         // 输入通道 AIN8+  AIN9-
	}
	else if(channel_select == 2)
	{
		ads1262_reg_write_buf[INPMUX] =     0x23;         // 输入通道 AIN2+  AIN3-
	}
	else if(channel_select == 3)	
	{
		ads1262_reg_write_buf[INPMUX] =     0x45;         // 输入通道 AIN4+  AIN5-
	}
	else if(channel_select == 4)
	{
		ads1262_reg_write_buf[INPMUX] =     0x67;         // 输入通道 AIN6+  AIN7-
	}
	
	ads1262_reg_write_buf[REFMUX] =     0x09;        // 000: Internal 2.5 V reference - P (default)  09
	
	//把准备好的值写回到ads1262中
	ads1262_write_register(ID,21,&ads1262_reg_write_buf[ID]);
	
//	// 清零 ads1262_reg_read_buf
//	memset(ads1262_reg_read_buf,0,strlen((const char*)ads1262_reg_read_buf));
//	// 读取寄存器
//	ads1262_read_register(ID,21,ads1262_reg_read_buf); //  保存读取出来的值
//	for(u8 i = 0 ;i < 21 ; i++)
//	{
//		usart1_printf("SET_register[0X%02X] :%02X\r\n",i,ads1262_reg_read_buf[i]);
//	}
//	usart1_printf("***********************************************\r\n");
	
	systick_delay_ms(10);     // 延时
	
	ads1262_start_hd_adc1();  // 启动转换
	
	for(u8 i= 0;i < 20; i++)
	{
		ads1262_CMD_SYOCAL1();    // 系统偏移自校准
		systick_delay_ms(800);    // 延时800 
		ads1262_stop_hd_adc1();   // 停止转换
		systick_delay_us(7) ;     // 等待停止
		syocal_1_data = 0;
		memset(ads1262_reg_read_buf,0,sizeof((const char*)ads1262_reg_read_buf));
		ads1262_read_register(OFCAL0,3,ads1262_reg_read_buf); //  保存读取出来的值
		usart1_printf("CH%d",channel_select);   // 显示通道信息
		for( u8 j = 0; j < 3 ; j++)
		{
			m++ ;
			usart1_printf(" > OFCAL[%d] = 0X%02X ",j+OFCAL0,ads1262_reg_read_buf[j]);
			sprintf((char *)string_buf," > Running [ %.0f ]%%",(double)((double)m/60.0)*100);  // 百分比字符串
			if(channel_select == 1)
			{
				lcd_fill_color(3,60,320,76,Tan);    //清理字符
				display_ascii_string(3,60,8,(u8*)string_buf,Red,Tan);  // lcd上显示百分比字符串
			}
			else if(channel_select == 2)
			{
				lcd_fill_color(3,100,320,116,Tan);    //清理字符
				display_ascii_string(3,100,8,(u8*)string_buf,Red,Tan); // lcd上显示百分比字符串
			}
			else if(channel_select == 3)
			{
				lcd_fill_color(3,140,320,156,Tan);    //清理字符
				display_ascii_string(3,140,8,(u8*)string_buf,Red,Tan); // lcd上显示百分比字符串
			}
			else if(channel_select == 4)
			{
				lcd_fill_color(3,180,320,196,Tan);    //清理字符
				display_ascii_string(3,180,8,(u8*)string_buf,Red,Tan); // lcd上显示百分比字符串
			}
			
			
		}
		usart1_printf("\r\n");

		syocal_1_data |= (int32_t)(ads1262_reg_read_buf[2]<< 16)  ;
		syocal_1_data |= (int32_t)(ads1262_reg_read_buf[1]<< 8 ) ;
		syocal_1_data |= (int32_t)(ads1262_reg_read_buf[0]<< 0 ) ;
		syocal_1_cali_data_buf[i] = syocal_1_data ;
		usart1_printf("CH%d [%d] = 0X%X\r\n",channel_select,i,syocal_1_cali_data_buf[i]);
		
	}
	syocal_1_data = 0 ;  // 清零后用于加和
	syocal_1_data = ads1262_get_average(syocal_1_cali_data_buf, 20) ;  // 取20个数排序去大去小后平均
	usart1_printf("=========================================\r\n");
	
	if(channel_select == 1)
	{
		ch1_ofcal_global_calibration_buf[0] = syocal_1_data & 0xff ;
		ch1_ofcal_global_calibration_buf[1] = (syocal_1_data >> 8 ) & 0xff ;
		ch1_ofcal_global_calibration_buf[2] = (syocal_1_data >> 16 ) & 0xff ;
		
		for(u8 i = 0 ; i < 3;i++)
		{
			usart1_printf("CH1_OFCAL[%d] = %X\r\n",7+i,ch1_ofcal_global_calibration_buf[i]);
		}
		// 将系统偏移校准的 07  08  09 寄存器值保存flash
		w25q32_flash_autosector_write_with_erase((u8*) &ch1_ofcal_global_calibration_buf[0],0x0000003C,1,1); // ch1  07 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch1_ofcal_global_calibration_buf[1],0x0000003D,1,1); // ch1  08 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch1_ofcal_global_calibration_buf[2],0x0000003E,1,1); // ch1  09 寄存器的校准值
	}
	else if(channel_select == 2)
	{
		ch2_ofcal_global_calibration_buf[0] = syocal_1_data & 0xff ;
		ch2_ofcal_global_calibration_buf[1] = (syocal_1_data >> 8 ) & 0xff ;
		ch2_ofcal_global_calibration_buf[2] = (syocal_1_data >> 16 ) & 0xff ;
		
		for(u8 i = 0 ; i < 3;i++)
		{
			usart1_printf("CH2_OFCAL[%d] = %X\r\n",7+i,ch2_ofcal_global_calibration_buf[i]);
		}
		w25q32_flash_autosector_write_with_erase((u8*) &ch2_ofcal_global_calibration_buf[0],0x0000003F,1,1); // ch2  07 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch2_ofcal_global_calibration_buf[1],0x00000040,1,1); // ch2  08 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch2_ofcal_global_calibration_buf[2],0x00000041,1,1); // ch2  09 寄存器的校准值
	}
	else if(channel_select == 3)
	{
		ch3_ofcal_global_calibration_buf[0] = syocal_1_data & 0xff ;
		ch3_ofcal_global_calibration_buf[1] = (syocal_1_data >> 8 ) & 0xff ;
		ch3_ofcal_global_calibration_buf[2] = (syocal_1_data >> 16 ) & 0xff ;
		
		for(u8 i = 0 ; i < 3;i++)
		{
			usart1_printf("CH3_OFCAL[%d] = %X\r\n",7+i,ch3_ofcal_global_calibration_buf[i]);
		}
		w25q32_flash_autosector_write_with_erase((u8*) &ch3_ofcal_global_calibration_buf[0],0x00000042,1,1); // ch3  07 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch3_ofcal_global_calibration_buf[1],0x00000043,1,1); // ch3  08 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch3_ofcal_global_calibration_buf[2],0x00000044,1,1); // ch3  09 寄存器的校准值
	}
	else if(channel_select == 4)
	{
		ch4_ofcal_global_calibration_buf[0] = syocal_1_data & 0xff ;
		ch4_ofcal_global_calibration_buf[1] = (syocal_1_data >> 8 ) & 0xff ;
		ch4_ofcal_global_calibration_buf[2] = (syocal_1_data >> 16 ) & 0xff ;
		
		for(u8 i = 0 ; i < 3;i++)
		{
			usart1_printf("CH4_OFCAL[%d] = %X\r\n",7+i,ch4_ofcal_global_calibration_buf[i]);
		}
		w25q32_flash_autosector_write_with_erase((u8*) &ch4_ofcal_global_calibration_buf[0],0x00000045,1,1); // ch4  07 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch4_ofcal_global_calibration_buf[1],0x00000046,1,1); // ch4  08 寄存器的校准值
		w25q32_flash_autosector_write_with_erase((u8*) &ch4_ofcal_global_calibration_buf[2],0x00000047,1,1); // ch4  09 寄存器的校准值
	}
	
//	usart1_printf("\r\n");
}
/*******************************************************************
********************************************************************
* 名称：void  ads1262_adc1_cmd_read_data(u8 adc_channel,u8 read_data_number, double Threshold_value,u8 Threshold_test_number)
* 功能：根据制定的通道在转换adc数据并返回
* 参数：adc_channel ：指定的ADC通道
read_data_number :要读取的adc 电压的字节数
Threshold_l_value : 低阈值
Threshold_h_value ： 电流阈值 （单位： mA）
Threshold_test_number ：判断达到阈值标准的次数
* 返回值： 无
* 版本：v1
* 作者：matt
* 日期：2024-01-28
* 举例：
* 备注：
* 
*                         2,147,483,647
AD采样电压= (寄存器读取数据/2^31 -1) * (VREF/PGA GAIN)
max49918   : I = (vout/gain) / 0.1R
*********************************************************************
*********************************************************************/
void  ads1262_adc1_cmd_read_data(u8 adc_channel,u8 read_data_number, double Threshold_l_value,double Threshold_h_value,u8 Threshold_test_number)
{
	int32_t  value_source = 0;                           // 保存读取出来的adc源码
	int32_t  value_temp[AVERAGE_DATA_LENG] = {0} ;       // 保存读取出来的电压寄存器的值
	double voltage_display = 0.0;                           // 保存ADC转换后的电压数据值
	double save_compare_current_A = 0.0;                  // 用于保存读取到的电流值，和电流的阈值比较
	double save_compare_current_mA = 0.0;                  // 用于保存读取到的电流值，和电流的阈值比较
	double save_compare_current_uA = 0.0;                  // 用于保存读取到的电流值，和电流的阈值比较

	double save_arithmetic_display_current_A =  0.0 ;   // 保存二次校准以后的电流值 a
	double save_arithmetic_display_current_mA =  0.0 ;   // 保存二次校准以后的电流值 ma
	double save_arithmetic_display_current_uA =  0.0 ;  // 保存二次校准以后的电流值 ua
	u8 adc_data[6] = {0};                                // 保存一次读取的5个字节中的adc值

	// 定义一个联合体
	union
	{
		int32_t voltage;
		struct
		{
			unsigned char frist;
			unsigned char second;
			unsigned char three;
			unsigned char four;
		} byte;
	} volt;
	
	
	ads1262_input_channel_select(adc_channel);     // 切换转换通道
	ads1262_start_cmd_adc1();                      // 开始adc转换

	
	for(u8 j = 0 ;j < AVERAGE_DATA_LENG; j++)       // 这里的循环值，表示循环读取的adc的数据的个数
	{
		wait_for_ads1262_adc1_conversion_ready();      // 等待转换数据ok
	
		spi2_read_write_byte(CMD_READ_DATA);           // 读adc数据，每次读取都要发命令   
		for(u8 i = 0;i < read_data_number; i++)        // 循环读取转换好的数据，放入数组中
		{
			adc_data[i] = spi2_read_write_byte(0x00); 
		}

		
		volt.byte.frist = adc_data[4];
		volt.byte.second= adc_data[3];
		volt.byte.three = adc_data[2];
		volt.byte.four  = adc_data[1];
		value_temp[j] = volt.voltage  ;
	}
	value_source = ads1262_get_average(value_temp,AVERAGE_DATA_LENG); // 处理读取AVERAGE_DATA_LENG个adc数据去掉大小值后取平均
	if(value_source  & 0x80000000 ) // 负数
	{
		value_source = (~value_source) & 0xFFFFFFFF;
		voltage_display = -(((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);
		
	}
	else                           // 正数
	{
		 voltage_display = (((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);
	}
	
	// ******************************通道1**********************************
	if(adc_channel == INPUT_CHANNEL_1)
	{
		save_compare_current_A = (voltage_display/ch1_test_range_value / SENSE_RESISTOR_CH1 ) - 0.000051 ;        // 保存安值，用于后面比较
		save_compare_current_mA = ((voltage_display*1000)/ch1_test_range_value / SENSE_RESISTOR_CH1) - 0.051  ; // 保存毫安值，用于后面比较
		save_compare_current_uA = ((voltage_display*1000000)/ch1_test_range_value / SENSE_RESISTOR_CH1) - 51 ;  // 保存微安值，用于后面比较
		// save_compare_current_A = (voltage_display/ch1_test_range_value / SENSE_RESISTOR_CH1 ) ;        // 保存安值，用于后面比较
		// save_compare_current_mA = ((voltage_display*1000)/ch1_test_range_value / SENSE_RESISTOR_CH1)   ; // 保存毫安值，用于后面比较
		// save_compare_current_uA = ((voltage_display*1000000)/ch1_test_range_value / SENSE_RESISTOR_CH1)  ;  // 保存微安值，用于后面比较

		save_arithmetic_display_current_A = calculate_linear_fit(save_compare_current_A, calibration_abc[0], calibration_abc[1]) ;     // 带入校准系数后的值
		save_arithmetic_display_current_mA = calculate_linear_fit(save_compare_current_mA, calibration_abc[0], calibration_abc[1]) ;   // 带入校准系数后的值
		save_arithmetic_display_current_uA = calculate_linear_fit(save_compare_current_uA, calibration_abc[0], calibration_abc[1]) ;   // 带入校准系数后的值

		

		if( (voltage_display  ) >= 1 ) // 1V
		{
			usart1_printf("[ < **CH1** > voltage = %.6Lf%s ]\r\n",voltage_display," V"); // 显示V ,A
		
			sprintf((char *)string_voltage,"%.6f%s",save_compare_current_A ," A"); // 经过调整系数后的电流值
			lcd_fill_color(44,72,227,96,Cyan);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,72,12,string_voltage,Blue,Cyan);

			usart1_printf("[ < **CH1** > current = %.6Lf%s ]\r\n",save_compare_current_A," A"); // 显示电流
			usart1_printf("[ < **CH1** > arithmetic current = %.6Lf%s ]\r\n",save_arithmetic_display_current_A," A"); // 显示电流
		}
		else if( (voltage_display * 1000 ) >= 1) // 将V 变换为  mV
		{
			usart1_printf("[ < **CH1** > voltage = %.3Lf%s ]\r\n",voltage_display * 1000," mV"); // 显示mV ,mA
			
			sprintf((char *)string_voltage,"%.3f%s",save_compare_current_mA," mA");
			lcd_fill_color(44,72,227,96,Cyan);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,72,12,string_voltage,Blue,Cyan);

			usart1_printf("[ < **CH1** > current = %.3Lf%s ]\r\n",save_compare_current_mA," mA"); // 显示电流
			usart1_printf("[ < **CH1** > arithmetic current = %.3Lf%s ]\r\n",save_arithmetic_display_current_mA," mA"); // 显示电流
		}
		else   // 将V 变换为  uV
		{
			usart1_printf("[ < **CH1** > voltage = %.1Lf%s ]\r\n",voltage_display * 1000000," uV"); // 显示uV ,uA

			sprintf((char *)string_voltage,"%.1f%s",save_compare_current_uA," uA");
			lcd_fill_color(44,72,227,96,Cyan);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,72,12,string_voltage,Blue,Cyan);

			usart1_printf("[ < **CH1** > current = %.1Lf%s ]\r\n",save_compare_current_uA," uA"); // 显示电流
			usart1_printf("[ < **CH1** > arithmetic current = %.1Lf%s ]\r\n",save_arithmetic_display_current_uA," uA"); // 显示电流
		}
		usart1_printf("***********************CH1************************\r\n");
		
		// ===========================================ch1根据阈值判断显示测试结论(阈值是mA)====================================================
		if( (save_compare_current_mA <= Threshold_h_value) && (Threshold_l_value <= save_compare_current_mA)) // 判断 L 阈值  <  电流值  <= H 阈值   （成功）
		{
			threshold_ch1_global_flag ++ ;                          // <= 阈值时，计数次数 +1
			if(threshold_ch1_global_flag >= Threshold_test_number) // 计数的次数和要求的次数相等时，显示绿色块
			{
				lcd_fill_color(230,41,311,122,Green);   // 显示测试ok
				display_ascii_string(258,69,12,(u8*)"OK",Black,Green);         // CH1 显示ok字符 X+28  Y+28

				ch1_test_circle_global_flag = false ;         //ch1 测试结束标志
				channel_4_test_over_global_flag |= 0x01 ;     // ch1 测试有结论后将BIT 0 置为=1

				if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
				{
					if(channel_global_flag == 0)              // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
					{
						if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
						{
							power_ctrl_ext(3,false);  // 关闭 3.3v电源
							power_ctrl_ext(5,false);  // 关闭 5v  电源
							time3_run(false);         // 关闭定时器
						}
					}
					else if(ch1_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
					{
						power_ctrl_ext(3,false);  // 关闭 3.3v电源
						power_ctrl_ext(5,false);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器
					}
				}
			}
		}
		else // 判断电流值  > 阈值时  （失败）
		{
			lcd_fill_color(230,41,311,122,Red);   // 显示测试失败
			display_ascii_string(258,69,12,(u8*)"NG",Black,Red);          // CH1 显示NG字符 X+28  Y+28
			ch1_test_circle_global_flag = false ;  // ch1测试结束标准
			channel_4_test_over_global_flag |= 0x01 ;    // ch1 测试有结论后将BIT 0 置为=1

			if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
			{
				if(channel_global_flag == 0) // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
				{
					if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
					{
						power_ctrl_ext(3,false);  // 关闭 3.3v电源
						power_ctrl_ext(5,false);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器
					}
				}
				else if(ch1_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
				{
					power_ctrl_ext(3,false);  // 关闭 3.3v电源
					power_ctrl_ext(5,false);  // 关闭 5v  电源
					time3_run(false);         // 关闭定时器
				}
			}
		}
		// ===========================================ch1根据阈值判断显示测试结论====================================================
	}
	// ******************通道2***********************
	else if( adc_channel == INPUT_CHANNEL_2 )
	{
		save_compare_current_A = (voltage_display/ch2_test_range_value / SENSE_RESISTOR_CH2 )- 0.0000155 ;        // 保存安值，用于后面比较
		save_compare_current_mA = ((voltage_display*1000)/ch2_test_range_value / SENSE_RESISTOR_CH2) - 0.0155  ; // 保存毫安值，用于后面比较
		save_compare_current_uA = ((voltage_display*1000000)/ch2_test_range_value / SENSE_RESISTOR_CH2) - 15.5 ;  // 保存微安值，用于后面比较

		if( (voltage_display  ) >= 1 ) // 1V
		{
			usart1_printf("[ < **CH2** > voltage = %.6Lf%s ]\r\n",voltage_display," V"); // 显示V ,A
		
			sprintf((char *)string_voltage,"%.6f%s",save_compare_current_A ," A");
			lcd_fill_color(44,167,227,191,Purple);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,167,12,string_voltage,Blue,Purple);	
		}
		else if( (voltage_display * 1000 ) >= 1) // 将V 变换为  mV
		{
			usart1_printf("[ < **CH2** > voltage = %.3Lf%s ]\r\n",voltage_display * 1000," mV"); // 显示mV ,mA
			
			sprintf((char *)string_voltage,"%.3f%s",save_compare_current_mA," mA");
			lcd_fill_color(44,167,227,191,Purple);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,167,12,string_voltage,Blue,Purple);	
		}
		else   // 将V 变换为  uV
		{
			usart1_printf("[ < **CH2** > voltage = %.1Lf%s ]\r\n",voltage_display * 1000000," uV"); // 显示uV ,uA

			sprintf((char *)string_voltage,"%.1f%s",save_compare_current_uA," uA");
			lcd_fill_color(44,167,227,191,Purple);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,167,12,string_voltage,Blue,Purple);	
		}
		usart1_printf("***********************CH2************************\r\n");
		// ============================ch2根据阈值判断显示测试结论(阈值是mA)=====================================
		
		if( (save_compare_current_mA <= Threshold_h_value) && (Threshold_l_value <=  save_compare_current_mA)) // 判断 L 阈值  <  电流值  <= H 阈值
		{
			threshold_ch2_global_flag ++ ; // <= 阈值时，计数次数 +1
			if(threshold_ch2_global_flag >= Threshold_test_number) // 计数的次数和要求的次数相等时，显示绿色块
			{
				lcd_fill_color(230,136,311,217,Green);   // 显示测试ok
				display_ascii_string(258,164,12,(u8*)"OK",Black,Green);         // CH2显示ok字符  X+28  Y+28
				ch2_test_circle_global_flag = false ;         //ch2 测试结束标准
				channel_4_test_over_global_flag |= 0x02 ;    // ch2 测试有结论后将BIT 1 置为=1

				if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
				{
					if(channel_global_flag == 0) // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
					{
						if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
						{
							power_ctrl_ext(3,true);  // 关闭 3.3v电源
							power_ctrl_ext(5,true);  // 关闭 5v  电源
							time3_run(false);         // 关闭定时器
						}
					}
					else if(ch2_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
					{
						power_ctrl_ext(3,true);  // 关闭 3.3v电源
						power_ctrl_ext(5,true);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器					
					}
				}
			}
		}
		else // 判断电流值  > 阈值时
		{
			lcd_fill_color(230,136,311,217,Red);   // 显示测试失败
			display_ascii_string(258,164,12,(u8*)"NG",Black,Red);          // CH2 显示NG字符  X+28  Y+28
			ch2_test_circle_global_flag = false ;  // ch1测试结束标准
			channel_4_test_over_global_flag |= 0x02 ;    // ch2 测试有结论后将BIT 1 置为=1
			if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
			{
				if(channel_global_flag == 0) // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
				{
					if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
					{
						power_ctrl_ext(3,false);  // 关闭 3.3v电源
						power_ctrl_ext(5,false);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器
					}
				}
				else if(ch2_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
				{
					power_ctrl_ext(3,false);  // 关闭 3.3v电源
					power_ctrl_ext(5,false);  // 关闭 5v  电源
					time3_run(false);         // 关闭定时器
				}
			}
		}
		// ============================ch2根据阈值判断显示测试结论=====================================
	}
	
	// 通道3
	else if( adc_channel == INPUT_CHANNEL_3 )
	{
		save_compare_current_A = (voltage_display/ch3_test_range_value / SENSE_RESISTOR_CH3 )- 0.0000365 ;        // 保存安值，用于后面比较
		save_compare_current_mA = ((voltage_display*1000)/ch3_test_range_value / SENSE_RESISTOR_CH3) - 0.0365  ; // 保存毫安值，用于后面比较
		save_compare_current_uA = ((voltage_display*1000000)/ch3_test_range_value / SENSE_RESISTOR_CH3) - 36.5 ;  // 保存微安值，用于后面比较

		if( (voltage_display  ) >= 1 ) // 1V
		{
			usart1_printf("[ < **CH3** > voltage = %.6Lf%s ]\r\n",voltage_display," V"); // 显示V ,A
		
			sprintf((char *)string_voltage,"%.6f%s",save_compare_current_A ," A");
			lcd_fill_color(44,262,227,286,MossGreen);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,262,12,string_voltage,Blue,MossGreen);
		}
		else if( (voltage_display * 1000 ) >= 1) // 将V 变换为  mV
		{
			usart1_printf("[ < **CH3** > voltage = %.3Lf%s ]\r\n",voltage_display * 1000," mV"); // 显示mV ,mA
			
			sprintf((char *)string_voltage,"%.3f%s",save_compare_current_mA," mA");
			lcd_fill_color(44,262,227,286,MossGreen);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,262,12,string_voltage,Blue,MossGreen);
		}
		else   // 将V 变换为  uV
		{
			usart1_printf("[ < **CH3** > voltage = %.1Lf%s ]\r\n",voltage_display * 1000000," uV"); // 显示uV ,uA

			sprintf((char *)string_voltage,"%.1f%s",save_compare_current_uA," uA");
			lcd_fill_color(44,262,227,286,MossGreen);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,262,12,string_voltage,Blue,MossGreen);
		}
		usart1_printf("***********************CH3************************\r\n");
		// ============================ch3根据阈值判断显示测试结论(阈值是mA)=====================================
		
		if( (save_compare_current_mA <= Threshold_h_value) && (Threshold_l_value <=  save_compare_current_mA)) // 判断 L 阈值  <  电流值  <= H 阈值
		{
			threshold_ch3_global_flag ++ ; // <= 阈值时，计数次数 +1
			if(threshold_ch3_global_flag >= Threshold_test_number) // 计数的次数和要求的次数相等时，显示绿色块
			{
				lcd_fill_color(230,231,311,312,Green);  // 显示测试ok
				display_ascii_string(258,259,12,(u8*)"OK",Black,Green);         // CH3显示ok字符  X+28  Y+28
				ch3_test_circle_global_flag = false ;  // ch3测试结束标准
				channel_4_test_over_global_flag |= 0x04 ;    // ch3 测试有结论后将BIT 2 置为=1
				if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
				{
					if(channel_global_flag == 0) // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
					{
						if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
						{
							power_ctrl_ext(3,false);  // 关闭 3.3v电源
							power_ctrl_ext(5,false);  // 关闭 5v  电源
							time3_run(false);         // 关闭定时器
						}
					}
					else if(ch3_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
					{
						power_ctrl_ext(3,false);  // 关闭 3.3v电源
						power_ctrl_ext(5,false);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器
					}
				}
			}
		}
		else // 判断电流值  > 阈值时
		{
			lcd_fill_color(230,231,311,312,Red);   // 显示测试失败
			display_ascii_string(258,259,12,(u8*)"NG",Black,Red);          // CH3 显示NG字符  X+28  Y+28
			ch3_test_circle_global_flag = false ;  // ch3测试结束标准
			channel_4_test_over_global_flag |= 0x04 ;    // ch3 测试有结论后将BIT 2 置为=1
			if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
			{
				if(channel_global_flag == 0) // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
				{
					if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
					{
						power_ctrl_ext(3,false);  // 关闭 3.3v电源
						power_ctrl_ext(5,false);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器
					}
				}
				else if(ch3_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
				{
					power_ctrl_ext(3,false);  // 关闭 3.3v电源
					power_ctrl_ext(5,false);  // 关闭 5v  电源
					time3_run(false);         // 关闭定时器
				}
			}
		}
		// ============================ch3根据阈值判断显示测试结论=====================================
	}
	// 通道4
	else if( adc_channel == INPUT_CHANNEL_4 )
	{
		save_compare_current_A = (voltage_display/ch4_test_range_value / SENSE_RESISTOR_CH4 )- 0.0000365 ;        // 保存安值，  用于后面比较
		save_compare_current_mA = ((voltage_display*1000)/ch4_test_range_value / SENSE_RESISTOR_CH4) - 0.0365  ;  // 保存毫安值，用于后面比较
		save_compare_current_uA = ((voltage_display*1000000)/ch4_test_range_value / SENSE_RESISTOR_CH4) - 36.5 ;  // 保存微安值，用于后面比较

		if( (voltage_display  ) >= 1 ) // 1V
		{
			usart1_printf("[ < **CH4** > voltage = %.6Lf%s ]\r\n",voltage_display," V"); // 显示V ,A
		
			sprintf((char *)string_voltage,"%.6f%s",save_compare_current_A ," A");
			lcd_fill_color(44,357,227,381,SlateBlue);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,357,12,string_voltage,Blue,SlateBlue);
		}
		else if( (voltage_display * 1000 ) >= 1) // 将V 变换为  mV
		{
			usart1_printf("[ < **CH4** > voltage = %.3Lf%s ]\r\n",voltage_display * 1000," mV"); // 显示mV ,mA
			
			sprintf((char *)string_voltage,"%.3f%s",save_compare_current_mA," mA");
			lcd_fill_color(44,357,227,381,SlateBlue);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,357,12,string_voltage,Blue,SlateBlue);
		}
		else   // 将V 变换为  uV
		{
			usart1_printf("[ < **CH4** > voltage = %.1Lf%s ]\r\n",voltage_display * 1000000," uV"); // 显示uV ,uA

			sprintf((char *)string_voltage,"%.1f%s",save_compare_current_uA," uA");
			lcd_fill_color(44,357,227,381,SlateBlue);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,357,12,string_voltage,Blue,SlateBlue);
		}
		usart1_printf("***********************CH4************************\r\n");
		
		// ============================ch4根据阈值判断显示测试结论(阈值是mA)=====================================
		
		if( (save_compare_current_mA <= Threshold_h_value) && (Threshold_l_value <=  save_compare_current_mA)) // 判断 L 阈值  <  电流值  <= H 阈值
		{
			threshold_ch4_global_flag ++ ; // <= 阈值时，计数次数 +1
			if(threshold_ch4_global_flag >= Threshold_test_number) // 计数的次数和要求的次数相等时，显示绿色块
			{
				lcd_fill_color(230,326,311,407,Green);   // 显示测试ok
				display_ascii_string(258,354,12,(u8*)"OK",Black,Green);         // CH4显示ok字符  X+28  Y+28
				ch4_test_circle_global_flag = false ;         //ch4测试结束标准
				channel_4_test_over_global_flag |= 0x08 ;    // ch4 测试有结论后将BIT 3 置为=1
				if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
				{
					if(channel_global_flag == 0) // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
					{
						if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
						{
							power_ctrl_ext(3,true);  // 关闭 3.3v电源
							power_ctrl_ext(5,true);  // 关闭 5v  电源
							time3_run(false);         // 关闭定时器
						}
					}
					else if(ch4_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
					{
						power_ctrl_ext(3,false);  // 关闭 3.3v电源
						power_ctrl_ext(5,false);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器
					}
				}
			}
		}
		else // 判断电流值  > 阈值时
		{
			lcd_fill_color(230,326,311,407,Red);    // 显示测试失败
			display_ascii_string(258,354,12,(u8*)"NG",Black,Red);          // CH4 显示NG字符  X+28  Y+28
			ch4_test_circle_global_flag = false ;  // ch4测试结束标准
			channel_4_test_over_global_flag |= 0x08 ;    // ch4 测试有结论后将BIT 3 置为=1
			if(continuous_test_mode_global_flag == false) //单次测试模式的时候，测试完毕过后关闭电源
			{
				if(channel_global_flag == 0) // 表示目前是全部通道打开的状态，这个时候要关闭电源的话，就要等4通道测试都结束
				{
					if((channel_4_test_over_global_flag & 0x0F) == 0x0F) // 判断4个通道是不是都测试完毕了 (电源是一个电源，4通道的时候一定要等全部通道测试完后才能关电源)
					{
						power_ctrl_ext(3,false);  // 关闭 3.3v电源
						power_ctrl_ext(5,false);  // 关闭 5v  电源
						time3_run(false);         // 关闭定时器
					}
				}
				else if(ch4_test_circle_global_flag == false)// 表示其他单通道模式，判断对应测试完成标志后，可以直接关闭电源，不用等其他通道
				{
					power_ctrl_ext(3,false);  // 关闭 3.3v电源
					power_ctrl_ext(5,false);  // 关闭 5v  电源
					time3_run(false);         // 关闭定时器
				}
			}
		}
		
		// ============================ch4根据阈值判断显示测试结论=====================================
	}
	

	
		
}

/*******************************************************************
********************************************************************
* 名称：void ads1262_read_data_auto_range_select(u8 adc_channel)
* 功能：通过选择的通道进行自动量程的选择（20ma/ 200ma量程切换）
向使用20ma量程测试，如果超过20ma量程就切换为200ma量程，如果还是超过200ma量程
显示提示信息，并关闭测试电源
* 参数：adc_channel ： 通道选择
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
#if 0
void ads1262_read_data_auto_range_select(u8 adc_channel)
{
	int32_t  value_source = 0;                             // 保存读取出来的adc源码
	int32_t  value_temp[AVERAGE_DATA_LENG] = {0} ;         // 保存读取出来的电压寄存器的值
	double   voltage_display= 0;                           // 保存ADC转换后的电压数据值
	u8       adc_data[6] = {0};                            // 保存一次读取的5个字节中的adc值

	// 定义一个联合体
	union
	{
		int32_t voltage;
		struct
		{
			unsigned char frist;
			unsigned char second;
			unsigned char three;
			unsigned char four;
		} byte;
	} volt;
	
	
	ads1262_input_channel_select(adc_channel);     // 切换转换通道并取相应通道系统校准值

	
	// ****************************************************************
	systick_delay_ms(50);
	ads1262_start_cmd_adc1();                      // 开始adc转换
	//================过滤不稳定数据=========================
	for(u16 j = 0 ;j < 100; j++)       // 这里的循环值，表示循环读取的adc的数据的个数 30*20
	{
		wait_for_ads1262_adc1_conversion_ready();      // 等待转换数据ok
		spi2_read_write_byte(CMD_READ_DATA);           // 读adc数据，每次读取都要发命令  
		spi2_read_write_byte(0x00); 		
//		for(u8 i = 0;i < 5; i++)        // 循环读取转换好的数据，放入数组中
//		{
//			adc_data[i] = spi2_read_write_byte(0x00); 
//		}

//		
//		volt.byte.frist = adc_data[4];
//		volt.byte.second= adc_data[3];
//		volt.byte.three = adc_data[2];
//		volt.byte.four  = adc_data[1];
	}
	//================过滤不稳定数据=========================
	memset((void*)adc_data,0,sizeof(adc_data));
	for(u8 j =0 ;j < AVERAGE_DATA_LENG; j++)       // 这里的循环值，表示循环读取的adc的数据的个数
	{
		wait_for_ads1262_adc1_conversion_ready();      // 等待转换数据ok
		spi2_read_write_byte(CMD_READ_DATA);           // 读adc数据，每次读取都要发命令   
		for(u8 i = 0;i < 5; i++)        // 循环读取转换好的数据，放入数组中
		{
			adc_data[i] = spi2_read_write_byte(0x00); 
		}

		
		volt.byte.frist = adc_data[4];
		volt.byte.second= adc_data[3];
		volt.byte.three = adc_data[2];
		volt.byte.four  = adc_data[1];
		value_temp[j] = volt.voltage  ;
	}
	value_source = ads1262_get_average(value_temp,AVERAGE_DATA_LENG); // 处理读取AVERAGE_DATA_LENG个adc数据去掉大小值后取平均
	if(value_source  & 0x80000000 ) // 负数
	{
		value_source = (~value_source) & 0xFFFFFFFF;
		voltage_display = -(((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);
		
	}
	else                           // 正数
	{
		 voltage_display = (((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);
	}
	usart1_printf("range 20mA >> voltage_display = [ %08f ] V\r\n",voltage_display); // 打印电压值

	if(voltage_display > 0.2)  // 在10R  的采样电阻下，采集到0.2v电压，表示电源降低了0.2v时，切换采样电阻为1R  (超过20MA 量程时)
	{
		usart1_printf("range 20mA >> over range voltage_display = [ %08f ] V\r\n",voltage_display); // 打印电压值
	
		//==============================================================================
		value_source = 0 ; // 	清零 ，为下一次准备
		memset((void*)value_temp,0,sizeof(value_temp));
		for(u8 j = 0 ;j < AVERAGE_DATA_LENG; j++)       // 这里的循环值，表示循环读取的adc的数据的个数
		{
			wait_for_ads1262_adc1_conversion_ready();      // 等待转换数据ok
			spi2_read_write_byte(CMD_READ_DATA);           // 读adc数据，每次读取都要发命令   
			for(u8 i = 0;i < 5; i++)        // 循环读取转换好的数据，放入数组中
			{
				adc_data[i] = spi2_read_write_byte(0x00); 
			}

			
			volt.byte.frist = adc_data[4];
			volt.byte.second= adc_data[3];
			volt.byte.three = adc_data[2];
			volt.byte.four  = adc_data[1];
			value_temp[j] = volt.voltage  ;
		}
		value_source = ads1262_get_average(value_temp,AVERAGE_DATA_LENG); // 处理读取AVERAGE_DATA_LENG个adc数据去掉大小值后取平均
		if(value_source  & 0x80000000 ) // 负数
		{
			value_source = (~value_source) & 0xFFFFFFFF;
			voltage_display = -(((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);
			
		}
		else                           // 正数
		{
			 voltage_display = (((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);
		}
		usart1_printf("200mA >> voltage_display = [ %08f ] V\r\n",voltage_display); // 打印电压值
		if(voltage_display > 0.2)  // 在采样电阻1R 下，压降0.2v时 （超过200MA 量程时）
		{
			usart1_printf("200mA >> over range voltage_display = [ %08f ] V\r\n",voltage_display); // 打印电压值
			if(adc_channel == INPUT_CHANNEL_1)
			{
				lcd_fill_color(44,72,227,96,Cyan);   // 覆盖上一次显示的字符，为下一次显示准备
				display_ascii_string(44,72,8,(unsigned char*)"OVER RANGE ERROR",Red,Cyan);
				lcd_fill_color(230,41,311,122,Red);   // 显示测试失败
				display_ascii_string(258,69,12,(u8*)"NG",Black,Red);          // CH1 显示NG字符 X+28  Y+28
				
			}
			else if(adc_channel == INPUT_CHANNEL_2)
			{
				lcd_fill_color(44,167,227,191,Purple);   // 覆盖上一次显示的字符，为下一次显示准备
				display_ascii_string(44,167,8,(unsigned char*)"OVER RANGE ERROR",Red,Purple);
				lcd_fill_color(230,136,311,217,Red);   // 显示测试失败
				display_ascii_string(258,164,12,(u8*)"NG",Black,Red);          // CH2 显示NG字符  X+28  Y+28
				
			}
			else if(adc_channel == INPUT_CHANNEL_3)
			{
				lcd_fill_color(44,262,227,286,MossGreen);   // 覆盖上一次显示的字符，为下一次显示准备
				display_ascii_string(44,262,8,(unsigned char*)"OVER RANGE ERROR",Red,MossGreen);
				lcd_fill_color(230,231,311,312,Red);   // 显示测试失败
				display_ascii_string(258,259,12,(u8*)"NG",Black,Red);          // CH3 显示NG字符  X+28  Y+28
				
			}
			else if(adc_channel == INPUT_CHANNEL_4)
			{
				lcd_fill_color(44,357,227,381,SlateBlue);   // 覆盖上一次显示的字符，为下一次显示准备
				display_ascii_string(44,357,8,(unsigned char*)"OVER RANGE ERROR",Red,SlateBlue);
				lcd_fill_color(230,326,311,407,Red);    // 显示测试失败
				display_ascii_string(258,354,12,(u8*)"NG",Black,Red);          // CH4 显示NG字符  X+28  Y+28
				
			}
			power_ctrl_ext(3,false);  // 关闭电源
            power_ctrl_ext(5,false);  // 关闭电源
			
		}
		else // 没有超过200ma 的量程时执行
		{
			if(adc_channel == INPUT_CHANNEL_1)
			{
				ch1_auto_200ma_range_select_global_flag = true ;  // ch1 没有超过量程的时候为真
			}
			else if(adc_channel == INPUT_CHANNEL_2)
			{
				ch2_auto_200ma_range_select_global_flag = true ;  // ch2 没有超过量程的时候为真
			}
			else if(adc_channel == INPUT_CHANNEL_3)
			{
				ch3_auto_200ma_range_select_global_flag = true ;  // ch3 没有超过量程的时候为真
			}
			else if(adc_channel == INPUT_CHANNEL_4)
			{
				ch4_auto_200ma_range_select_global_flag = true ;  // ch4 没有超过量程的时候为真
			}
			
			display_ascii_string(114,459,6,(u8*)"[   200 mA   ]",Black,Tan); // 显示量程200ma
		                                         
		}
		
		
	}
	else// 没有超过20MA 量程时
	{
		if(adc_channel == INPUT_CHANNEL_1)
		{
			ch1_auto_20ma_range_select_global_flag = true ;  // ch1 没有超过量程的时候为真
		}
		else if(adc_channel == INPUT_CHANNEL_2)
		{
			ch2_auto_20ma_range_select_global_flag = true ;  // ch1 没有超过量程的时候为真
		}
		else if(adc_channel == INPUT_CHANNEL_3)
		{
			ch3_auto_20ma_range_select_global_flag = true ;  // ch1 没有超过量程的时候为真
		}
		else if(adc_channel == INPUT_CHANNEL_4)
		{
			ch4_auto_20ma_range_select_global_flag = true ;  // ch1 没有超过量程的时候为真
		}
		display_ascii_string(114,459,6,(u8*)"[   20 mA    ]",Black,Tan); // 显示量程200ma
	}
	
	

}
#endif

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void ads1262_auto_range_select(u8 adc_channel)
{
	int32_t  value_source = 0;                             // 保存读取出来的adc源码
	int32_t  value_temp[AVERAGE_DATA_LENG] = {0} ;         // 保存读取出来的电压寄存器的值
	double   voltage_display= 0;                           // 保存ADC转换后的电压数据值
	u8       adc_data[6] = {0};                            // 保存一次读取的5个字节中的adc值
	u8       range_flag = 0;                               // 用于量程选择中的循环计数
	// 定义一个联合体
	union
	{
		int32_t voltage;
		struct
		{
			unsigned char frist;
			unsigned char second;
			unsigned char three;
			unsigned char four;
		} byte;
	} volt;
	
	
	ads1262_input_channel_select(adc_channel);     // 切换转换通道并取相应通道系统校准值

	
	// ****************************************************************
	systick_delay_ms(25);
	ads1262_start_cmd_adc1();                      // 开始adc转换
	systick_delay_ms(25);

	for( range_flag = 0 ;range_flag < 5; range_flag ++) // 6个增益选项循环判断量程
	{
		memset((void*)adc_data,0,sizeof(adc_data));      // 清零
		memset((void*)value_temp,0,sizeof(value_temp));  // 清零
		for(u8 j =0 ;j < 10; j++)         // 这里的循环值，表示循环读取的adc的数据的个数
		{
			wait_for_ads1262_adc1_conversion_ready();      // 等待转换数据ok
			spi2_read_write_byte(CMD_READ_DATA);           // 读adc数据，每次读取都要发命令   
			for(u8 i = 0;i < 5; i++)        // 循环读取转换好的数据，放入数组中
			{
				adc_data[i] = spi2_read_write_byte(0x00); 
			}
			
			volt.byte.frist = adc_data[4];
			volt.byte.second= adc_data[3];
			volt.byte.three = adc_data[2];
			volt.byte.four  = adc_data[1];
			value_temp[j] = volt.voltage  ;
		}
		value_source = ads1262_get_average(value_temp,10); // 处理读取AVERAGE_DATA_LENG个adc数据去掉大小值后取平均

		if(value_source  & 0x80000000 ) // 负数
		{
			value_source = (~value_source) & 0xFFFFFFFF;
			voltage_display = -(((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);	
		}
		else                            // 正数
		{
			voltage_display = (((double)value_source) / 2147483647.0) * (double)(EXT_REF_VOLTAGE);
		}
		if(range_flag == 0) // 显示第一次的量程值 （默认增益200）
		{
			usart1_printf("DEFAULT RANGE [125mA] \r\n");
		}
		usart1_printf(" >> range select voltage_display = [ %08f ] V\r\n",voltage_display); // 打印电压值
		
		if(voltage_display >= 2.5)  // 超过量程2.5v （默认是增益200 小量程开始测试）
		{
			if(range_flag == 0)       // for循环第1次循环
			{ 
				display_ascii_string(114,459,6,(u8*)"[   156 mA   ]",Black,Tan); // 显示量程156ma
				if(adc_channel == INPUT_CHANNEL_1)
				{
					max49918_set_gain(I2C1,U28_MAX49918_WRITE_ADDR,GAIN_160);  // 设置增益160 
					ch1_test_range_value = 160 ;
					usart1_printf("CH1 : RANGE [156mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_2)
				{
					max49918_set_gain(I2C1,U29_MAX49918_WRITE_ADDR,GAIN_160);  // 设置增益160 
					ch2_test_range_value = 160 ;
					usart1_printf("CH2 : RANGE [156mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_3)
				{
					max49918_set_gain(I2C3,U30_MAX49918_WRITE_ADDR,GAIN_160);  // 设置增益160 
					ch3_test_range_value = 160 ;
					usart1_printf("CH3 : RANGE [156mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_4)
				{
					max49918_set_gain(I2C3,U31_MAX49918_WRITE_ADDR,GAIN_160);  // 设置增益160 
					ch4_test_range_value = 160 ;
					usart1_printf("CH4 : RANGE [156mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}	  
			}
			else if (range_flag == 1) // for循环第2次循环   
			{
				display_ascii_string(114,459,6,(u8*)"[   250 mA   ]",Black,Tan); // 显示量程250ma
				if(adc_channel == INPUT_CHANNEL_1)
				{
					max49918_set_gain(I2C1,U28_MAX49918_WRITE_ADDR,GAIN_100);  // 设置增益100
					ch1_test_range_value = 100 ;
					usart1_printf("CH1 : RANGE [250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_2)
				{
					max49918_set_gain(I2C1,U29_MAX49918_WRITE_ADDR,GAIN_100);  // 设置增益100
					ch2_test_range_value = 100 ;
					usart1_printf("CH2 : RANGE [250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_3)
				{
					max49918_set_gain(I2C3,U30_MAX49918_WRITE_ADDR,GAIN_100);  // 设置增益100
					ch3_test_range_value = 100 ;
					usart1_printf("CH3 : RANGE [250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_4)
				{
					max49918_set_gain(I2C3,U31_MAX49918_WRITE_ADDR,GAIN_100);  // 设置增益100
					ch4_test_range_value = 100 ;
					usart1_printf("CH4 : RANGE [250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
			}
			else if (range_flag == 2)  // for循环第3次循环
			{
				display_ascii_string(114,459,6,(u8*)"[   312 mA   ]",Black,Tan); // 显示量程312ma
				if(adc_channel == INPUT_CHANNEL_1)
				{
					max49918_set_gain(I2C1,U28_MAX49918_WRITE_ADDR,GAIN_80);  // 设置增益80
					ch1_test_range_value = 80 ;
					usart1_printf("CH1 : RANGE [312mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_2)
				{
					max49918_set_gain(I2C1,U29_MAX49918_WRITE_ADDR,GAIN_80);  // 设置增益80
					ch2_test_range_value = 80 ;
					usart1_printf("CH2 : RANGE [312mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_3)
				{
					max49918_set_gain(I2C3,U30_MAX49918_WRITE_ADDR,GAIN_80);  // 设置增益80
					ch3_test_range_value = 80 ;
					usart1_printf("CH3 : RANGE [312mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_4)
				{
					max49918_set_gain(I2C3,U31_MAX49918_WRITE_ADDR,GAIN_80);  // 设置增益80
					ch4_test_range_value = 80 ;
					usart1_printf("CH4 : RANGE [312mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
			}
			else if (range_flag == 3)  // for循环第4次循环
			{
				display_ascii_string(114,459,6,(u8*)"[   625 mA   ]",Black,Tan); // 显示量程625ma
				if(adc_channel == INPUT_CHANNEL_1)
				{
					max49918_set_gain(I2C1,U28_MAX49918_WRITE_ADDR,GAIN_40);  // 设置增益40
					ch1_test_range_value = 40;
					usart1_printf("CH1 : RANGE [625mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_2)
				{
					max49918_set_gain(I2C1,U29_MAX49918_WRITE_ADDR,GAIN_40);  // 设置增益40
					ch2_test_range_value = 40;
					usart1_printf("CH2 : RANGE [625mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_3)
				{
					max49918_set_gain(I2C3,U30_MAX49918_WRITE_ADDR,GAIN_40);  // 设置增益40
					ch3_test_range_value = 40;
					usart1_printf("CH3 : RANGE : RANGE [625mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_4)
				{
					max49918_set_gain(I2C3,U31_MAX49918_WRITE_ADDR,GAIN_40);  // 设置增益40
					ch4_test_range_value = 40;
					usart1_printf("CH4 : RANGE [625mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
			}
			else if (range_flag == 4)   // for循环第5次循环
			{
				display_ascii_string(114,459,6,(u8*)"[   1250mA   ]",Black,Tan); // 显示量程1250ma
				if(adc_channel == INPUT_CHANNEL_1)
				{
					max49918_set_gain(I2C1,U28_MAX49918_WRITE_ADDR,GAIN_20);  // 设置增益20
					ch1_test_range_value = 20;
					usart1_printf("CH1 : RANGE [1250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_2)
				{
					max49918_set_gain(I2C1,U29_MAX49918_WRITE_ADDR,GAIN_20);  // 设置增益20
					ch2_test_range_value = 20;
					usart1_printf("CH2 : RANGE [1250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_3)
				{
					max49918_set_gain(I2C3,U30_MAX49918_WRITE_ADDR,GAIN_20);  // 设置增益20
					ch3_test_range_value = 20;
					usart1_printf("CH3 : RANGE[1250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
				else if(adc_channel == INPUT_CHANNEL_4)
				{
					max49918_set_gain(I2C3,U31_MAX49918_WRITE_ADDR,GAIN_20);  // 设置增益20
					ch4_test_range_value = 20;
					usart1_printf("CH4 : RANGE [1250mA] (%d)\r\n",range_flag);
					systick_delay_us(50);
				}
			}
		}
		else  // 第一次循环 增益200时，没有超过量程
		{
			if(range_flag == 0)
			{
				display_ascii_string(114,459,6,(u8*)"[   125 mA   ]",Black,Tan); // 显示量程125ma
				if(adc_channel == INPUT_CHANNEL_1)
				{
					usart1_printf("CH1 : RANGE 125 ma\r\n");
					ch1_test_range_value = 200 ;
				}
				else if(adc_channel == INPUT_CHANNEL_2)
				{
					usart1_printf("CH2 : RANGE 125 ma\r\n");
					ch2_test_range_value = 200 ;
				}
				else if(adc_channel == INPUT_CHANNEL_3)
				{
					usart1_printf("CH3 : RANGE 125 ma\r\n");
					ch3_test_range_value = 200 ;
				}
				else if(adc_channel == INPUT_CHANNEL_4)
				{
					usart1_printf("CH4 : RANGE 125 ma\r\n");
					ch4_test_range_value = 200 ;
				}

				
			}
			break; // 退出for 循环
			
		}

	}
	if(range_flag >= 5)  // 表示所有增益都超量程了
	{
		power_ctrl_ext(3,false);  // 关闭电源
        power_ctrl_ext(5,false);  // 关闭电源
		
		if(adc_channel == INPUT_CHANNEL_1)
		{
			usart1_printf("CH1 OVER RANGE ERROR\r\n");
			lcd_fill_color(44,72,227,96,Cyan);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,72,8,(unsigned char*)"OVER RANGE ERROR",Red,Cyan);
			lcd_fill_color(230,41,311,122,Red);   // 显示测试失败
			display_ascii_string(258,69,12,(u8*)"NG",Black,Red);          // CH1 显示NG字符 X+28  Y+28
			
		}
		else if(adc_channel == INPUT_CHANNEL_2)
		{
			usart1_printf("CH2 OVER RANGE ERROR\r\n");
			lcd_fill_color(44,167,227,191,Purple);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,167,8,(unsigned char*)"OVER RANGE ERROR",Red,Purple);
			lcd_fill_color(230,136,311,217,Red);   // 显示测试失败
			display_ascii_string(258,164,12,(u8*)"NG",Black,Red);          // CH2 显示NG字符  X+28  Y+28
			
		}
		else if(adc_channel == INPUT_CHANNEL_3)
		{
			usart1_printf("CH3 OVER RANGE ERROR\r\n");
			lcd_fill_color(44,262,227,286,MossGreen);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,262,8,(unsigned char*)"OVER RANGE ERROR",Red,MossGreen);
			lcd_fill_color(230,231,311,312,Red);   // 显示测试失败
			display_ascii_string(258,259,12,(u8*)"NG",Black,Red);          // CH3 显示NG字符  X+28  Y+28
			
		}
		else if(adc_channel == INPUT_CHANNEL_4)
		{
			usart1_printf("CH4 OVER RANGE ERROR\r\n");
			lcd_fill_color(44,357,227,381,SlateBlue);   // 覆盖上一次显示的字符，为下一次显示准备
			display_ascii_string(44,357,8,(unsigned char*)"OVER RANGE ERROR",Red,SlateBlue);
			lcd_fill_color(230,326,311,407,Red);    // 显示测试失败
			display_ascii_string(258,354,12,(u8*)"NG",Black,Red);          // CH4 显示NG字符  X+28  Y+28
			
		}
		
	}
	else  // 没有超过量程的时候进行测试
	{
		if(adc_channel == INPUT_CHANNEL_1)
		{
			ch1_auto_range_select_global_flag = true ;  // ch1 自动量程选择标识，没有超量程为真，初始化为假
		}
		else if(adc_channel == INPUT_CHANNEL_2)
		{
			ch2_auto_range_select_global_flag = true ;	// ch2 自动量程选择标识，没有超量程为真，初始化为假
		}
		else if(adc_channel == INPUT_CHANNEL_3)
		{
			ch3_auto_range_select_global_flag = true ;	// ch3 自动量程选择标识，没有超量程为真，初始化为假
		}
		else if(adc_channel == INPUT_CHANNEL_4)
		{
			ch4_auto_range_select_global_flag = true ;	// ch4 自动量程选择标识，没有超量程为真，初始化为假
		}

	}


}

