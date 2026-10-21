#ifndef __ADS1262_H__
#define __ADS1262_H__
#include "stm32f10x.h"
#include "io_bit.h"
#include "stdbool.h"
//===========================================================================
#define EXT_REF_VOLTAGE  5
#define INT_REF_VOLTAGE  5
//===========================================================================
#define ADS1262_RESET        PEout(3)   // PE3   RESET
#define ADS1262_CS           PBout(12)  // PB12  CS  CS-仅确定串行接口，不控制RST和START信号
#define ADS1262_READY        PEin(4)    // PE4   ready


//==============================cmd=============================================
#define CMD_RESET        0X06                             //复位adc命令
#define CMD_START_ADC1   0X08                             //开始ADC1 转换命令
#define CMD_STOP_ADC1    0X0A                             //停止ADC1 转换命令
#define CMD_READ_DATA    0X12                             //读adc1数据命令
#define CMD_SYOCAL1      0X16                             //adc1系统偏移校准
#define CMD_SYGCAL1      0X17                             //adc1系统增益校准
#define CMD_SFOCAL1      0X19                             //adc1自偏移校准 命令      
#define CMD_READ_REG     0X20                             //读寄存器命令
#define CMD_WRITE_REG    0X40                             //写读寄存器命令

//=========================寄存器地址==================================================
//OFCAL[2:0]与FSCAL[2:0]寄存器组。OFCAL[2:0]与FSCAL[2:0]用于ADS1262偏移和满量程校正，
//ADS1262可根据OFCAL[2:0]与FSCAL[2:0]中的校正系数对数据进行自动校正

//INTERFACE寄存器。用户通过配置INTERFACE寄存器中INTERFACE.2和INTERFACE.0位决定数据帧长度，
//即是否输出STATUS状态字节和CRC字节，数据帧最长为6字节，最短为4字节，STATUS字节包含当前输出数
//据是否曾被读取，PGA输出是否溢出，ADC复位状态等重要信息。

//当滤波模式选择sinc3、A/D采样率设置为1200 S/s且开启斩波自稳0模式时，输出数据最稳定
#define ID									(0x00)			//芯片id号
#define POWER 								(0x01)			// 内部参考电压及复位状态显示
#define INTERFACE							(0x02)			//数据中status字节及crc字节的选择
#define MODE0								(0x03)			//测量模式，自稳零模式与启动转换延时选择
#define MODE1								(0x04)			//数字滤波模式选择及传感器偏置电流源配置
#define MODE2								(0x05)			//PGA增益 系数与adc采样数率选择
#define INPMUX								(0x06)			//模拟新年好输入的多路选择
#define OFCAL0								(0x07)			//校正后，存储adc起始偏移值
#define OFCAL1								(0x08)			
#define OFCAL2								(0x09)			
#define FSCAL0								(0x0A)			// 校正后，存储满量程增益系数值
#define FSCAL1								(0x0B)            
#define FSCAL2								(0x0C)
#define IDACMUX								(0x0D)          //双激励电流源模拟输入多路选择
#define IDACMAG								(0x0E)          //idac1  idac2的激励电流大小选择
#define REFMUX								(0x0F)          //外部参考电压的输入多路选择与极性设置
#define TDACP								(0x10)          //TDACP引脚连接及测试电压大小的选择
#define TDACN								(0x11)          //TDACN引脚连接及测试电压大小的选择
#define GPIOCON								(0x12)          // gpio口引脚连接设置
#define GPIODIR								(0x13)          // gpio口引脚输入或输出选择
#define GPIODAT								(0x14)          //gpio口电平状态显示
	
//===========================================================================	
void get_ads1262_id(void);
void ads1262_hd_reset(void);
void ads1262_soft_reset(void);
void ads1262_start_adc1(void);
void ads1262_stop_adc1(void);
u32 ads1262_read_adc1_conversion_data(void);
void ads1262_init(void);
void ads1262_read_register(u8 start_address,u8 num_reg,u8* p_data);	 
void ads1262_read_direct_adc1_conversion_data(u8 num_bytes,u8 data_bytes_start_num);
void wait_for_ads1262_adc1_conversion_ready(void);

#endif


