#include "ads1262.h"
#include "stm32f10x.h"
#include "spi2.h"
#include "systick_delay.h"
#include "stdio.h"
#include "stdbool.h"
#include "lcd.h"
//===========================================================================
unsigned char string_buf2[100];   // 用于sprintf函数打印字符串的u32
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
pe3  reset   
pe4  ready
pb12 cs

*********************************************************************
*********************************************************************/
static void ads1262_ctrl_pin_init(void)
{
	GPIO_InitTypeDef GPIO_InitTypeDef_pin;
	// 打开GPIOB  GPIOE 的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	// PE3  reset
	GPIO_InitTypeDef_pin.GPIO_Mode =  GPIO_Mode_Out_PP;
	GPIO_InitTypeDef_pin.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitTypeDef_pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_pin);
	// PE4  ready
	GPIO_InitTypeDef_pin.GPIO_Mode =  GPIO_Mode_IPU; //=================?????
	GPIO_InitTypeDef_pin.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitTypeDef_pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_pin);
	// PB12  cs
//	GPIO_InitTypeDef_pin.GPIO_Mode =  GPIO_Mode_Out_PP;
//	GPIO_InitTypeDef_pin.GPIO_Pin = GPIO_Pin_12;
//	GPIO_InitTypeDef_pin.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB,&GPIO_InitTypeDef_pin);
	
//	GPIO_SetBits(GPIOB,GPIO_Pin_12);    // cs 拉高，不片选
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);  // cs 拉低，片选
	  
	
	

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
	ADS1262_CS = 0;                                      // 片选
	spi2_read_write_byte(CMD_READ_REG + start_address);  // 先发送ads1262的读取命令
	spi2_read_write_byte(num_reg - 1);                   // 在发送读取寄存器的个数
	for(i=0;i<num_reg;i++)
	{
		p_data[i] = spi2_read_write_byte(0x00);
	}
	ADS1262_CS = 1;                                     // 不片选
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
	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_WRITE_REG + start_address);  // 先发送ads1262的写命令
	spi2_read_write_byte(num_reg - 1);                    // 在发送写几个寄存器的数量
	for(i=0;i<num_reg;i++)
	{
		spi2_read_write_byte(p_data[i]);                  // 发送要写的内容
	}
	ADS1262_CS = 1;                                       // 不片选
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
	ads1262_read_register(ID, 1,&data);  
	if(data & 0xe0)// 000 
	{
		printf("ID:ADS1263\r\n");  // 表示1263
	}
	else
	{
		printf("ID:ADS1262\r\n");  // 表示1262
	}
	
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
void ads1262_hd_reset(void)
{
	ADS1262_RESET = 0 ;
	systick_delay_ms(6);  //拉低的时间不能太长，超过9ms，进入掉电模式，在掉电模式中退出，拉高要等待9ms
	ADS1262_RESET = 1 ;
	systick_delay_ms(50);
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
	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_RESET);
	ADS1262_CS = 1;                                       // 不片选
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
void ads1262_start_adc1(void)
{
	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_START_ADC1);
	ADS1262_CS = 1;                                       // 不片选
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
	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_STOP_ADC1);
	ADS1262_CS = 1;                                       // 不片选
}
void ads1262_CMD_SFOCAL1(void)
{
	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_SFOCAL1);
	ADS1262_CS = 1; 
}

void ads1262_CMD_SYGCAL1(void)
{
	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_SYGCAL1);
	ADS1262_CS = 1; 
}

void ads1262_CMD_SYOCAL1(void)
{
	ADS1262_CS = 0;                                       // 片选
	spi2_read_write_byte(CMD_SYOCAL1);
	ADS1262_CS = 1; 
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
	while (!ADS1262_READY) ; 
	while (ADS1262_READY) ; 	
}


/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：num_bytes : 返回的adc数据字节数，4,5,6可选
       data_bytes_start_num  ：真正的adc数据开始于第几个字节，可选0,1
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-02-08
* 举例：
* 备注：
* 
* adc1的数据是二进制补码格式保存的
*********************************************************************
*********************************************************************/
void ads1262_read_direct_adc1_conversion_data(u8 num_bytes,u8 data_bytes_start_num)
{
	u8 INPMUX_temp = 0;                                             //用于保存寄存器的值，比较修改的值是否真正的修改完成
	u8 INPMUX_chanel2 = 0x98;                                          //用于改变通道的寄存器值
	u8 INPMUX_chanel1 = 0x54;                                          //用于改变通道的寄存器值
	u8 adc_data[6],i ;                                               // 保存读取出来的adc值  i 用于循环
	int32_t adc_data_source = 0;                                     // 保存最终的adc原数据
	double voltage;                                                 // 保存转换后的数据值
	
	ADS1262_CS = 0;                                                 // 片选
	wait_for_ads1262_adc1_conversion_ready();                       // 等待数据ok
    spi2_read_write_byte(CMD_READ_DATA);                            // 读adc数据，每次读取都要发命令                                     
	for(i=0;i < num_bytes; i++)                                     // 循环读取转换好的数据，放入数组中
	{
		adc_data[i] = spi2_read_write_byte(0x00); 
//		printf("0x%X\r\n",adc_data[i]);
	}
//	printf("=================\r\n");
	adc_data_source |=  (u32)(adc_data[data_bytes_start_num + 0] << 24);   // MSB
	adc_data_source |=  (u32)(adc_data[data_bytes_start_num + 1] << 16);   // MSB
	adc_data_source |=  (u32)(adc_data[data_bytes_start_num + 2] << 8 );   // MSB
	adc_data_source |=  (u32)(adc_data[data_bytes_start_num + 3] << 0 );   // LSB
	ADS1262_CS = 1;                                                      // 不片选
//	printf("adc_data_source = 0X%X\r\n",adc_data_source);
	if(adc_data_source & 0x80000000)                                       // 判断最高符号位 0： 表示正数 1：表示负数
	{
		// 处理负数
		adc_data_source = 0-((adc_data_source ^ 0xffffffff) +1);           //负数取反后 +1 ，在添符号就是原值
		printf("adc_change = %d\r\n",adc_data_source);
	}
	else
	{
		// 处理正数
		printf("adc_change = %d\r\n",adc_data_source);
	}
	voltage  = (double)(((double)(adc_data_source) / 0x7fffffff) * INT_REF_VOLTAGE); 
	
//	printf("[VOLTAGE =  %.6Lf]\r\n",voltage * 4 *4);                  //使用了分压网络，测得的电压缩小了4倍的，真正电压得x4
	sprintf((char *)string_buf2,"%.8Lf%s",voltage * 4 *4*2," V");
	display_ascii_string(29,37,12,string_buf2,Red,Tan);

	

//	while(1)
//	{
//		ads1262_stop_adc1();                                            //停止转换，要更新寄存器值
//		ads1262_write_register(INPMUX,1,&INPMUX_chanel2);                 // AIN9输入+  /AIN8输入-
//		ads1262_read_register(INPMUX,1,&INPMUX_temp);
//		if(INPMUX_temp == INPMUX_chanel2)
//		{
//			ads1262_start_adc1();   // 开始adc1 的转换
//			break;                  // 跳出while（1）
//		}
//	}
	

	
	
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

	u8 ads1262_reg_read_buf[21];                       //  ads1262 有21个寄存器 ,保存读出来的寄存器值
	u8 ads1262_reg_write_buf[21];                      //  ads1262 有21个寄存器 ,保存要写的寄存器值
	ads1262_read_register(ID,21,ads1262_reg_read_buf); //  保存读取出来的值

	
	//改变要写入寄存器的值
	ads1262_reg_write_buf[ID] = ads1262_reg_read_buf[ID] ;
	ads1262_reg_write_buf[POWER] =      0x10;         // 复位1，偏置关闭，内部参考关闭 1 开内部  0 关内部参考
	ads1262_reg_write_buf[INTERFACE] =  0x00;         // 关闭串口超时，关闭状态字节，关闭校验（4个字节数据）0x06  6字节  00  4字节
	ads1262_reg_write_buf[MODE0] =      0x90;         // 正的参考电压极性，连续转换模式，斩波使能，转换0延时  0101 0000
	                                                  // 单次转换模式下，开始时redy 为1，数据ok后为低
	ads1262_reg_write_buf[MODE1] =      0x60;         // sinC4模式0x60    sinc3： 0x40
	ads1262_reg_write_buf[MODE2] =      0x00;         // PGA使能=1，1200sps   1000 1001    pga =1， 10sps  0000 0010
	ads1262_reg_write_buf[INPMUX] =     0x54;         // AIN5输入+  /AIN4 acom输入-  // 0xcc  逻辑电源输入
//	ads1262_reg_write_buf[OFCAL0] =
//	ads1262_reg_write_buf[OFCAL1] =
//	ads1262_reg_write_buf[OFCAL2] =
//	ads1262_reg_write_buf[FSCAL0] =
//	ads1262_reg_write_buf[FSCAL1] =
//	ads1262_reg_write_buf[FSCAL2] =
	ads1262_reg_write_buf[REFMUX] =      0x09;       // 外部参考 AIN0+   AIN1-

	
	//把准备好的值写回到ads1262中
	ads1262_write_register(ID,21,ads1262_reg_write_buf);
	ads1262_CMD_SFOCAL1();
	ads1262_CMD_SYGCAL1();
	ads1262_CMD_SYOCAL1();
	
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
	spi2_init();                           // 通信接口spi2 初始化 1
	ads1262_ctrl_pin_init();               // 其他控制引脚初始化  片选
	ads1262_hd_reset();                    // 复位
	ads1262_register_init();               // 寄存器初始化  
		 
}


