#include "spi1.h"
#include "stm32f4xx.h"
#include <stdio.h> 
//===========================================================================
uint8_t SPI1_FLASH_BUF[4096];
/*******************************************************************
********************************************************************
* 名称：static void spi1_pin_init(u8 mode)
* 功能：初始化spi1
* 参数：mode spi的通信模式 0-3
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* PA4:CS  PA5:SCK PA6 :MISO  PA7:MOSI
*********************************************************************
*********************************************************************/
static void spi1_pin_init(u8 mode)
{
	GPIO_InitTypeDef  GPIO_InitTypeDef_spi1;
	SPI_InitTypeDef   SPI_InitTypeDef_SPI1;
	
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOA  C时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI1时钟
	// sck  miso  mosi
	GPIO_InitTypeDef_spi1.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitTypeDef_spi1.GPIO_Mode =  GPIO_Mode_AF;//复用功能
	GPIO_InitTypeDef_spi1.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitTypeDef_spi1.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitTypeDef_spi1.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_spi1); //初始化
	
	//配置引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_SPI1); //PA5复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_SPI1); //PA6复用为 SPI1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_SPI1); //PA7复用为 SPI1
	
	
	
	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);
	// miso

	
	// CS pa4  u20位号
	GPIO_InitTypeDef_spi1.GPIO_Pin = GPIO_Pin_4;     // pin4
	GPIO_InitTypeDef_spi1.GPIO_Mode =  GPIO_Mode_OUT;//输出
	GPIO_InitTypeDef_spi1.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitTypeDef_spi1.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitTypeDef_spi1.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_spi1);
	GPIO_SetBits(GPIOA,GPIO_Pin_4);// 拉高cs，不片选flash
	
	
	// CS pc1 (另一个flash )  u33位号

	GPIO_InitTypeDef_spi1.GPIO_Pin = GPIO_Pin_1;     // pin4
	GPIO_InitTypeDef_spi1.GPIO_Mode =  GPIO_Mode_OUT;//输出
	GPIO_InitTypeDef_spi1.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_InitTypeDef_spi1.GPIO_Speed = GPIO_Speed_100MHz;//100MHz
	GPIO_InitTypeDef_spi1.GPIO_PuPd = GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_spi1);
	GPIO_SetBits(GPIOC,GPIO_Pin_1);// 拉高cs，不片选flash
	
	//这里只针对 SPI 口初始化
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,ENABLE);//复位 SPI1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_SPI1,DISABLE);//停止复位 SPI1
	
	// SPI1 通信相关的参数设置
	SPI_InitTypeDef_SPI1.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;       //预分频 2
	if(mode == 0)
	{
		SPI_InitTypeDef_SPI1.SPI_CPHA              = SPI_CPHA_1Edge;                  //第1个跳变沿数据被采样
		SPI_InitTypeDef_SPI1.SPI_CPOL              = SPI_CPOL_Low;                   //串行同步时钟的空闲状态为低电平
	}
	else if(mode == 1)
	{
		SPI_InitTypeDef_SPI1.SPI_CPHA              = SPI_CPHA_2Edge;                  //第二个跳变沿数据被采样
		SPI_InitTypeDef_SPI1.SPI_CPOL              = SPI_CPOL_Low;                   //串行同步时钟的空闲状态为低电平
	}
	else if(mode == 2)
	{
		SPI_InitTypeDef_SPI1.SPI_CPHA              = SPI_CPHA_1Edge;                  //第1个跳变沿数据被采样
		SPI_InitTypeDef_SPI1.SPI_CPOL              = SPI_CPOL_High;                   //串行同步时钟的空闲状态为高电平
	}
	else if(mode == 3)
	{
		SPI_InitTypeDef_SPI1.SPI_CPHA              = SPI_CPHA_2Edge;                  //第二个跳变沿数据被采样
		SPI_InitTypeDef_SPI1.SPI_CPOL              = SPI_CPOL_High;                   //串行同步时钟的空闲状态为高电平
	}

	SPI_InitTypeDef_SPI1.SPI_CRCPolynomial     = 7;                               //CRC 值计算的多项式
	SPI_InitTypeDef_SPI1.SPI_DataSize          = SPI_DataSize_8b;                 // SPI 发送接收 8 位帧结构
	SPI_InitTypeDef_SPI1.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; //双线双向全双工
	SPI_InitTypeDef_SPI1.SPI_FirstBit          = SPI_FirstBit_MSB;                //数据传输从 MSB 位开始
	SPI_InitTypeDef_SPI1.SPI_Mode              = SPI_Mode_Master;                 //主 SPI
	SPI_InitTypeDef_SPI1.SPI_NSS               = SPI_NSS_Soft;                    //NSS 信号由软件控制
	
	SPI_Init(SPI1,&SPI_InitTypeDef_SPI1);                                         //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
	
	
	SPI_Cmd(SPI1, ENABLE);                                                        //使能 SPI 外设
	
}
/*******************************************************************
********************************************************************
* 名称：void SPI1_SetSpeed(u8 SpeedSet)
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
void SPI1_SetSpeed(u8 SpeedSet)
{
	SPI_InitTypeDef   SPI_InitTypeDef_SPI1;
	SPI_InitTypeDef_SPI1.SPI_BaudRatePrescaler = SpeedSet ;
	SPI_Init(SPI1, &SPI_InitTypeDef_SPI1);
	SPI_Cmd(SPI1,ENABLE);
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
void spi1_init(void)
{
	spi1_pin_init(3);
//	SPI1_SetSpeed(SPI_BaudRatePrescaler_16 ); //设置为18M时钟,高速模式，因为读写Flash数据要求速度比高。
}

/*******************************************************************
********************************************************************
* 名称：u8 spi1_read_write_byte(u8 data)
* 功能：spi1同时读写一个直接数据
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
//u8 spi1_read_write_byte(u8 data)
//{
//	u16 retry = 0;
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查并等待至TX缓冲区为空
//	{
//		retry++;
//		if(retry > 800)
//		{
//			return 0;
//		}
//	};
//	//程序执行到此处，TX缓冲区已空
//	SPI_I2S_SendData(SPI1, data); //通过外设 SPIx 发送一个数据
////	retry = 0;
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)//检查并等待至RX缓冲区为非空
//	{
//		retry++;
//		if(retry > 800)
//		{
//			return 0;
//		}
//	};
//	return SPI_I2S_ReceiveData(SPI1); //返回通过 SPI1 最近接收的数据
//}


u8 spi1_read_write_byte(u8 data)
{
	while((SPI1->SR &SPI_I2S_FLAG_TXE)==RESET);
	SPI1->DR = data;
	while((SPI1->SR &SPI_I2S_FLAG_RXNE)==RESET);
	return SPI1->DR; 
}



//u8 spi1_read_write_byte(u8 data)
//{
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){}//等待发送区空
//	SPI_I2S_SendData(SPI1, data); //通过外设 SPIx 发送一个 byte 数据
//	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET){} //等待接收完
//	return SPI_I2S_ReceiveData(SPI1); //返回通过 SPIx 最近接收的数据
//}


