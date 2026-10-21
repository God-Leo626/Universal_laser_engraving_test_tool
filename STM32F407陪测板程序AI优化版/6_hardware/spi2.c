#include "spi2.h"
#include "stm32f4xx.h"
#include <stdbool.h> 
//===========================================================================
u8 SPI2_FLASH_BUF[4096] = {0};
/*******************************************************************
********************************************************************
* 名称：static void spi2_pin_init(u8 mode)
* 功能：初始化spi2
* 参数：mode spi的通信模式 0-3
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* PB12 :CS  
  PB13 :SCK 
  PB14 :MISO  
  PB15 :MOSI
*********************************************************************
*********************************************************************/
static void spi2_pin_init(u8 mode)
{
	GPIO_InitTypeDef  GPIO_InitTypeDef_spi2;
	SPI_InitTypeDef   SPI_InitTypeDef_SPI2;
	
	
	SPI_StructInit(&SPI_InitTypeDef_SPI2);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//使能SPI2时钟

 
	//  PB13 :SCK 
	//  PB14 :MISO  
	//  PB15 :MOSI
	GPIO_InitTypeDef_spi2.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitTypeDef_spi2.GPIO_Mode =  GPIO_Mode_AF;//复用功能
	GPIO_InitTypeDef_spi2.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitTypeDef_spi2.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitTypeDef_spi2.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_spi2); //初始化
	
	//配置引脚复用映射
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_SPI2); //PB13 复用为 SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource14,GPIO_AF_SPI2); //PB14 复用为 SPI2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource15,GPIO_AF_SPI2); //PB15 复用为 SPI2
	
	
	
	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);
	// miso

	
	//  PB12 :CS 
	GPIO_InitTypeDef_spi2.GPIO_Pin = GPIO_Pin_12;     // pin12
	GPIO_InitTypeDef_spi2.GPIO_Mode =  GPIO_Mode_OUT;//输出
	GPIO_InitTypeDef_spi2.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitTypeDef_spi2.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitTypeDef_spi2.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_spi2);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);// 拉高cs，不片选
	
	

	
	//这里只针对 SPI 口初始化
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE);//复位 SPI2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);//停止复位 SPI2
	
	// SPI2 通信相关的参数设置
	SPI_InitTypeDef_SPI2.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;       //预分频 2
	if(mode == 0)
	{
		SPI_InitTypeDef_SPI2.SPI_CPHA              = SPI_CPHA_1Edge;                  //第1个跳变沿数据被采样
		SPI_InitTypeDef_SPI2.SPI_CPOL              = SPI_CPOL_Low;                   //串行同步时钟的空闲状态为低电平
	}
	else if(mode == 1)
	{
		SPI_InitTypeDef_SPI2.SPI_CPHA              = SPI_CPHA_2Edge;                  //第二个跳变沿数据被采样
		SPI_InitTypeDef_SPI2.SPI_CPOL              = SPI_CPOL_Low;                   //串行同步时钟的空闲状态为低电平
	}
	else if(mode == 2)
	{
		SPI_InitTypeDef_SPI2.SPI_CPHA              = SPI_CPHA_1Edge;                  //第1个跳变沿数据被采样
		SPI_InitTypeDef_SPI2.SPI_CPOL              = SPI_CPOL_High;                   //串行同步时钟的空闲状态为高电平
	}
	else if(mode == 3)
	{
		SPI_InitTypeDef_SPI2.SPI_CPHA              = SPI_CPHA_2Edge;                  //第二个跳变沿数据被采样
		SPI_InitTypeDef_SPI2.SPI_CPOL              = SPI_CPOL_High;                   //串行同步时钟的空闲状态为高电平
	}

	SPI_InitTypeDef_SPI2.SPI_CRCPolynomial     = 7;                               //CRC 值计算的多项式
	SPI_InitTypeDef_SPI2.SPI_DataSize          = SPI_DataSize_8b;                 // SPI 发送接收 8 位帧结构
	SPI_InitTypeDef_SPI2.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; //双线双向全双工
	SPI_InitTypeDef_SPI2.SPI_FirstBit          = SPI_FirstBit_MSB;                //数据传输从 MSB 位开始
	SPI_InitTypeDef_SPI2.SPI_Mode              = SPI_Mode_Master;                 //主 SPI
	SPI_InitTypeDef_SPI2.SPI_NSS               = SPI_NSS_Soft;                    //NSS 信号由软件控制
	
	SPI_Init(SPI2,&SPI_InitTypeDef_SPI2);                                         //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	
	
	SPI_Cmd(SPI2, ENABLE);                                                        //使能 SPI 外设
	
}
/*******************************************************************
********************************************************************
* 名称：void SPI2_SetSpeed(u8 SpeedSet)
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* //SPI 速度设置函数
//SpeedSet: 2、4、6、8、16、32、64、128、256
//SPI2、SPI3 是 APB1上的设备，最高通信速率为 36M/2 = 18Mbits/s
//SPI1 是 APB2上的设备，最高通信速率达 72M/2 = 36Mbtis/s
* 
*********************************************************************
*********************************************************************/
void SPI2_SetSpeed(u8 SpeedSet)
{
    u16 control;
    u32 retry = 1000000UL;
    if (!IS_SPI_BAUDRATE_PRESCALER(SpeedSet)) return;
    while ((SPI2->SR & SPI_I2S_FLAG_BSY) != 0)
        if (--retry == 0) return;
    /* 只修改BR分频位。原实现把其余未初始化结构体字段写入CR1，破坏模式。 */
    control = SPI2->CR1;
    SPI2->CR1 = control & (u16)~SPI_CR1_SPE;
    SPI2->CR1 = (control & (u16)~SPI_CR1_BR) | SpeedSet;
}

/*******************************************************************
********************************************************************
* 名称：void w25q32_init(void)
* 功能：初始化flash
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void spi2_init(void)
{
	spi2_pin_init(3); // 3
//	SPI1_SetSpeed(SPI_BaudRatePrescaler_16 ); //设置为18M时钟,高速模式，因为读写Flash数据要求速度比高。
}

/*******************************************************************
********************************************************************
* 名称：u8 spi2_read_write_byte(u8 data)
* 功能：spi2同时读写一个直接数据
* 参数：data：要发送的数据 一个字节
* 返回值：接收到的数据  一个字节
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

/* 字节等待有上限，故障时返回0xFF；上层通过状态接口区分真实0xFF与错误。 */
static bool transfer_ok = true;

bool spi2_last_transfer_ok(void) { return transfer_ok; }

u8 spi2_read_write_byte(u8 data)
{
    u32 retry = 1000000UL;
    transfer_ok = false;
    while ((SPI2->SR & SPI_I2S_FLAG_TXE) == 0)
        if (--retry == 0) return 0xff;
    SPI2->DR = data;
    retry = 1000000UL;
    while ((SPI2->SR & SPI_I2S_FLAG_RXNE) == 0)
        if (--retry == 0) return 0xff;
    transfer_ok = true;
    return (u8)SPI2->DR;
}
