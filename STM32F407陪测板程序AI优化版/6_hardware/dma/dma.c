#include "dma.h"
#include "stm32f10x.h"
#include "usart_dma_all.h"
//**************************************************************************************

DMA_InitTypeDef  DMA1_tx_Init;      // DMA 接收初始化结构体全局变量        
DMA_InitTypeDef  DMA1_rx_Init;      // DMA 发送初始化结构体全局变量  

DMA_InitTypeDef  DMA1_tx3_Init;      // DMA 接收初始化结构体全局变量        
DMA_InitTypeDef  DMA1_rx3_Init;      // DMA 发送初始化结构体全局变量 

DMA_InitTypeDef  DMA2_tx4_Init;      // DMA 接收初始化结构体全局变量        
DMA_InitTypeDef  DMA2_rx4_Init;      // DMA 发送初始化结构体全局变量 
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
* usart3---rx---dma-----channel 3
  usart3---tx---dma-----channel 2
  串口的中断优先级应低于串口DMA通道的中断优先级。
*********************************************************************
*********************************************************************/
void dma1_usart3_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);						//使能DMA1时钟

	//DMA_USART3_RX  USART3->RAM的数据传输
	DMA_DeInit(DMA1_Channel3);												//将DMA的通道3寄存器重设为缺省值 
	DMA1_rx3_Init.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);				//启动传输前装入实际RAM地址
	DMA1_rx3_Init.DMA_MemoryBaseAddr = (u32)USART3_RX_BUF_1;            	//设置接收缓冲区首地址
	DMA1_rx3_Init.DMA_DIR = DMA_DIR_PeripheralSRC;								//数据传输方向，从外设读取到内存
	DMA1_rx3_Init.DMA_BufferSize = USART3_REC_LEN;							   //DMA通道的DMA缓存的大小
	DMA1_rx3_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//外设地址寄存器不变
	DMA1_rx3_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;							//内存地址寄存器递增
	DMA1_rx3_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			//数据宽度为8位
	DMA1_rx3_Init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					//数据宽度为8位
	DMA1_rx3_Init.DMA_Mode = DMA_Mode_Normal;									//工作在正常模式
	DMA1_rx3_Init.DMA_Priority = DMA_Priority_VeryHigh; 							//DMA通道 x拥有高优先级 
	DMA1_rx3_Init.DMA_M2M = DMA_M2M_Disable;									//DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel3,&DMA1_rx3_Init); 									//对DMA通道3进行初始化
	
	//DMA_USART3_TX  RAM->USART3的数据传输
	DMA_DeInit(DMA1_Channel2);												//将DMA的通道2寄存器重设为缺省值 
	DMA1_tx3_Init.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);					//启动传输前装入实际RAM地址
	DMA1_tx3_Init.DMA_MemoryBaseAddr = (u32)USART3_TX_BUF;              		//设置发送缓冲区首地址
	DMA1_tx3_Init.DMA_DIR = DMA_DIR_PeripheralDST; 								//数据传输方向，从内存发送到外设
	DMA1_tx3_Init.DMA_BufferSize = USART3_TX_LEN;							//DMA通道的DMA缓存的大小
	DMA1_tx3_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//外设地址寄存器不变
	DMA1_tx3_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;							//内存地址寄存器递增
	DMA1_tx3_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			//数据宽度为8位
	DMA1_tx3_Init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					//数据宽度为8位
	DMA1_tx3_Init.DMA_Mode = DMA_Mode_Normal;									//工作在正常模式
	DMA1_tx3_Init.DMA_Priority = DMA_Priority_High; 							//DMA通道 x拥有高优先级 
	DMA1_tx3_Init.DMA_M2M = DMA_M2M_Disable;									//DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel2,&DMA1_tx3_Init); 									//对DMA通道2进行初始化
	
	//DMA1通道3 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;				//NVIC通道设置
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9 ;				//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;						//子优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);											//根据指定的参数初始化NVIC寄存器
 
	//DMA1通道2 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;				//NVIC通道设置
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9 ;				//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;						//子优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);											//根据指定的参数初始化NVIC寄存器

//	DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,ENABLE);							//开USART3 Rx DMA中断
//	DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,ENABLE);							//开USART3 Tx DMA中断

	USART_DMACmd(USART3, USART_DMAReq_Tx, ENABLE);        					//使能串口3 DMA发送
//	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);                          //使能串口3 DMA接收
	
	DMA_Cmd(DMA1_Channel3,DISABLE);           								//使DMA通道3停止工作 rx
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
* usart1---rx---dma-----channel 5
  usart1---tx---dma-----channel 4
  串口的中断优先级应低于串口DMA通道的中断优先级。
*********************************************************************
*********************************************************************/
void dma1_usart1_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);						//使能DMA1时钟

	//*****DMA_USART1_RX  USART1->RAM的数据传输******
	DMA_DeInit(DMA1_Channel5);												//将DMA的通道3寄存器重设为缺省值 
	DMA1_rx_Init.DMA_PeripheralBaseAddr = (u32)&(USART1->DR);					//启动传输前装入实际RAM地址
	DMA1_rx_Init.DMA_MemoryBaseAddr = (u32)USART1_RX_BUF_1;            		//设置接收缓冲区首地址
	DMA1_rx_Init.DMA_DIR = DMA_DIR_PeripheralSRC;								//数据传输方向，从外设读取到内存
	DMA1_rx_Init.DMA_BufferSize = USART1_REC_LEN;							    //DMA通道的DMA缓存的大小
	DMA1_rx_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//外设地址寄存器不变
	DMA1_rx_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;							//内存地址寄存器递增
	DMA1_rx_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			//数据宽度为8位
	DMA1_rx_Init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					//数据宽度为8位
	DMA1_rx_Init.DMA_Mode = DMA_Mode_Normal;									//工作在正常模式
	DMA1_rx_Init.DMA_Priority = DMA_Priority_VeryHigh; 							//DMA通道 x拥有高优先级 
	DMA1_rx_Init.DMA_M2M = DMA_M2M_Disable;									//DMA通道x没有设置为内存到内存传输 
	DMA_Init(DMA1_Channel5,&DMA1_rx_Init); 									//对DMA通道5进行初始化
	
	
	//******DMA_USART1_TX  RAM->USART1的数据传输*******
	DMA_DeInit(DMA1_Channel4);												//将DMA的通道2寄存器重设为缺省值 
	DMA1_tx_Init.DMA_PeripheralBaseAddr = (u32)&(USART1->DR);					//启动传输前装入实际RAM地址
	DMA1_tx_Init.DMA_MemoryBaseAddr = (u32)USART1_TX_BUF;              		//设置发送缓冲区首地址
	DMA1_tx_Init.DMA_DIR = DMA_DIR_PeripheralDST; 								//数据传输方向，从内存发送到外设
	DMA1_tx_Init.DMA_BufferSize = USART1_TX_LEN;							    //DMA通道的DMA缓存的大小
	DMA1_tx_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//外设地址寄存器不变
	DMA1_tx_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;							//内存地址寄存器递增
	DMA1_tx_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			//数据宽度为8位
	DMA1_tx_Init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					//数据宽度为8位
	DMA1_tx_Init.DMA_Mode = DMA_Mode_Normal;									//工作在正常模式
	DMA1_tx_Init.DMA_Priority = DMA_Priority_High; 							//DMA通道 x拥有高优先级 
	DMA1_tx_Init.DMA_M2M = DMA_M2M_Disable;									//DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel4,&DMA1_tx_Init); 									//对DMA通道4进行初始化
	
//	//DMA1通道3 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;				//NVIC通道设置
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9 ;				//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;						//子优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);											//根据指定的参数初始化NVIC寄存器
// 
//	//DMA1通道2 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;				//NVIC通道设置
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9 ;				//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;						//子优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);											//根据指定的参数初始化NVIC寄存器

//	DMA_ITConfig(DMA1_Channel5,DMA_IT_TC,ENABLE);							//开USART1 Rx DMA中断
//	DMA_ITConfig(DMA1_Channel4,DMA_IT_TC,ENABLE);							//开USART1 Tx DMA中断
	 
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);        					//开启串口DMA发送
//	USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);        					//开启串口DMA接收
	
	DMA_Cmd(DMA1_Channel5,DISABLE);           								//使DMA通道5停止工作 rx
//	DMA_Cmd(DMA1_Channel4,DISABLE);           								//使DMA通道4停止工作 tx
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
* uart4---rx---dma2-----channel 3
  uart4---tx---dma2-----channel 5
  串口的中断优先级应低于串口DMA通道的中断优先级。
*********************************************************************
*********************************************************************/
void dma2_uart4_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);						//使能DMA2时钟

	//DMA_UART4_RX  UART4->RAM的数据传输
	DMA_DeInit(DMA2_Channel3);												//将DMA的通道3寄存器重设为缺省值 
	DMA2_rx4_Init.DMA_PeripheralBaseAddr = (u32)(&UART4->DR);					//启动传输前装入实际RAM地址
	DMA2_rx4_Init.DMA_MemoryBaseAddr = (u32)UART4_RX_BUF_1;            				//设置接收缓冲区首地址
	DMA2_rx4_Init.DMA_DIR = DMA_DIR_PeripheralSRC;								//数据传输方向，从外设读取到内存
	DMA2_rx4_Init.DMA_BufferSize = UART4_REC_LEN;							   //DMA通道的DMA缓存的大小
	DMA2_rx4_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//外设地址寄存器不变
	DMA2_rx4_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;							//内存地址寄存器递增
	DMA2_rx4_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			//数据宽度为8位
	DMA2_rx4_Init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					//数据宽度为8位
	DMA2_rx4_Init.DMA_Mode = DMA_Mode_Normal;									//工作在正常模式
	DMA2_rx4_Init.DMA_Priority = DMA_Priority_VeryHigh; 							//DMA通道 x拥有高优先级 
	DMA2_rx4_Init.DMA_M2M = DMA_M2M_Disable;									//DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA2_Channel3,&DMA2_rx4_Init); 									//对DMA通道3进行初始化
	
	//DMA_UART4_TX  RAM->UART4的数据传输
	DMA_DeInit(DMA2_Channel5);												//将DMA的通道2寄存器重设为缺省值 
	DMA2_tx4_Init.DMA_PeripheralBaseAddr = (u32)(&UART4->DR);					//启动传输前装入实际RAM地址
	DMA2_tx4_Init.DMA_MemoryBaseAddr = (u32)UART4_TX_BUF;              		//设置发送缓冲区首地址
	DMA2_tx4_Init.DMA_DIR = DMA_DIR_PeripheralDST; 								//数据传输方向，从内存发送到外设
	DMA2_tx4_Init.DMA_BufferSize = UART4_TX_LEN;							//DMA通道的DMA缓存的大小
	DMA2_tx4_Init.DMA_PeripheralInc = DMA_PeripheralInc_Disable;				//外设地址寄存器不变
	DMA2_tx4_Init.DMA_MemoryInc = DMA_MemoryInc_Enable;							//内存地址寄存器递增
	DMA2_tx4_Init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;			//数据宽度为8位
	DMA2_tx4_Init.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;					//数据宽度为8位
	DMA2_tx4_Init.DMA_Mode = DMA_Mode_Normal;									//工作在正常模式
	DMA2_tx4_Init.DMA_Priority = DMA_Priority_High; 							//DMA通道 x拥有高优先级 
	DMA2_tx4_Init.DMA_M2M = DMA_M2M_Disable;									//DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA2_Channel5,&DMA2_tx4_Init); 									//对DMA通道2进行初始化
	
	//DMA1通道3 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;				//NVIC通道设置
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9 ;				//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;						//子优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);											//根据指定的参数初始化NVIC寄存器
 
	//DMA1通道2 NVIC 配置
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel2_IRQn;				//NVIC通道设置
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9 ;				//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;						//子优先级
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;							//IRQ通道使能
//	NVIC_Init(&NVIC_InitStructure);											//根据指定的参数初始化NVIC寄存器

//	DMA_ITConfig(DMA1_Channel3,DMA_IT_TC,ENABLE);							//开UART4 Rx DMA中断
//	DMA_ITConfig(DMA1_Channel2,DMA_IT_TC,ENABLE);							//开UART4 Tx DMA中断

	USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);        					//使能串口4 DMA发送
//	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);                          //使能串口4 DMA接收
	
	DMA_Cmd(DMA2_Channel3,DISABLE);           								//使DMA通道3停止工作 rx
}


