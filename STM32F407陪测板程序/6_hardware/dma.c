#include "dma.h"
#include "stm32f4xx.h"
#include "usart_dma_all.h"
#include "variable_global.h"
#include "string.h"
//**************************************************************************************
/*******************************************************************
********************************************************************
* 名称：void usart1_dma_tx_configuration(void)
* 功能：串口1的发送 dma配置
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-12-24
* 举例：
* 备注：
* 
* usart1  dma2  tx  数据流7  通道4
                rx  数据流2  通道4
*********************************************************************
*********************************************************************/
void usart1_dma_tx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;                                     //初始化结构体变量
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 , ENABLE);	//DMA2 时钟使能				
	
	DMA_DeInit(DMA2_Stream7);
	while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE){}; //等待 DMA 可配置						
	DMA_InitStructure.DMA_Channel = DMA_Channel_4; 	//设置 DMA 数据流对应的通道						
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART1_TX_BUF;	    //发送缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					//DMA传输方向：内存--->外设
	DMA_InitStructure.DMA_BufferSize = strlen((const char *)USART1_TX_BUF);		                //数据传输字节数量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式    DMA_Mode_Normal   DMA_Mode_Circular
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					//中等优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  // 禁用fifo
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	DMA_Init(DMA2_Stream7 , &DMA_InitStructure);							//初始化DMA Stream
	DMA_Cmd(DMA2_Stream7 , DISABLE); 										//不开启DMA传输
}
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
* 备注：
*  usart1  dma2  tx  数据流7  通道4
                 rx  数据流2  通道4
* 
*********************************************************************
*********************************************************************/
void usart1_dma_rx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
 
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 , ENABLE);					//DMA2时钟使能
	DMA_DeInit(DMA2_Stream2);
	while (DMA_GetCmdStatus(DMA2_Stream2) != DISABLE);						//等待DMA可配置  
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  						//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART1_RX_BUF;	    //接收缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;				//DMA传输方向：外设到存储器模式：外设--->内存
	DMA_InitStructure.DMA_BufferSize = USART1_REC_LEN;		                //缓冲大小 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式   DMA_Mode_Normal
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;					    //高优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	DMA_Init(DMA2_Stream2 , &DMA_InitStructure);							//初始化DMA_Stream5	
	DMA_Cmd(DMA2_Stream2 , ENABLE);  										//开启DMA传输
}
/*******************************************************************
********************************************************************
* 名称：void usart1_dma_tx_send(uint8_t *send_buffer , u32 nSendCount)
* 功能：串口1开始 dma发送 
* 参数： send_buffer：要发送的字符指针
        nSendCount:要发送的数据长度
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-12-24
* 举例：
* 备注：
* usart1  dma2   tx  数据流7  通道4
                 rx  数据流2  通道4
* 
*********************************************************************
*********************************************************************/

void usart1_dma_tx_send(uint8_t *send_buffer , u32 nSendCount)
{	
	if (nSendCount < USART1_TX_LEN)
	{
		memcpy(USART1_TX_BUF , send_buffer , nSendCount);
		DMA_Cmd(DMA2_Stream7 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE){};	//确保DMA可以被设置
        DMA2_Stream7->M0AR = (uint32_t)USART1_TX_BUF;       // 重置内存地址
		DMA_SetCurrDataCounter(DMA2_Stream7 , nSendCount);  //设置数据传输量
		DMA_Cmd(DMA2_Stream7 , ENABLE);               		//DMA使能,开始发送	
		
		while(DMA_GetFlagStatus(DMA2_Stream7,DMA_FLAG_TCIF7) == RESET) ;           // 等待发送完成
		
		
//		memset(USART1_TX_BUF,0,strlen((const char*)USART1_TX_BUF));
		DMA_Cmd(DMA2_Stream7 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA2_Stream7) != DISABLE){};	//确保DMA可以被设置
        
		DMA_SetCurrDataCounter(DMA2_Stream7 , 0);
		DMA_ClearFlag(DMA2_Stream7 , DMA_FLAG_TCIF7 | DMA_FLAG_FEIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_TEIF7 | DMA_FLAG_HTIF7);
	}
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：  USART2 DMA 发送配置 (DMA1_Stream6_Channel4)
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* USART2_TX	DMA1	Stream6	Channel4
  USART2_RX	DMA1	Stream5	Channel4
*********************************************************************
*********************************************************************/
void usart2_dma_tx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;                                     //初始化结构体变量
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 , ENABLE);	                //DMA1 时钟使能				
	
	DMA_DeInit(DMA1_Stream6);    
	while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE){};                    //等待 DMA 可配置						
	DMA_InitStructure.DMA_Channel = DMA_Channel_4; 	                        //设置 DMA 数据流对应的通道						
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART2_TX_BUF;	    //发送缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					//DMA传输方向：内存--->外设
	DMA_InitStructure.DMA_BufferSize = strlen((const char *)USART2_TX_BUF);	//数据传输字节数量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					//中等优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  // 禁用fifo
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	
    DMA_Init(DMA1_Stream6 , &DMA_InitStructure);							//初始化DMA Stream
	DMA_Cmd(DMA1_Stream6 , DISABLE); 										//不开启DMA传输
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：  USART2 DMA 接收配置 (DMA1_Stream5_Channel4)
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
*   USART2_TX	DMA1	Stream6	Channel4
    USART2_RX	DMA1	Stream5	Channel4
* 
*********************************************************************
*********************************************************************/
void usart2_dma_rx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
 
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 , ENABLE);					//DMA1时钟使能
	DMA_DeInit(DMA1_Stream5);
	while (DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);						//等待DMA可配置  
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  						//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART2_RX_BUF;	    //接收缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;				//DMA传输方向：外设到存储器模式：外设--->内存
	DMA_InitStructure.DMA_BufferSize = USART2_REC_LEN;		                //缓冲大小 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;					    //高优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;           //将 FIFO 阈值设置为 “满” 状态,意味着 DMA 在 FIFO 达到满状态时才会进行数据传输操作。
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	
    DMA_Init(DMA1_Stream5 , &DMA_InitStructure);							//初始化DMA_Stream5	
	DMA_Cmd(DMA1_Stream5 , ENABLE);  										//开启DMA传输
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
*   USART2_TX	DMA1	Stream6	Channel4
    USART2_RX	DMA1	Stream5	Channel4
*********************************************************************
*********************************************************************/
void usart2_dma_tx_send(uint8_t *send_buffer , u32 nSendCount)
{	
	if (nSendCount < USART2_TX_LEN)
	{
		memcpy(USART2_TX_BUF , send_buffer , nSendCount);
		DMA_Cmd(DMA1_Stream6 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE){};	//确保DMA可以被设置
        DMA1_Stream6->M0AR = (uint32_t)USART2_TX_BUF;    //  重置内存
		DMA_SetCurrDataCounter(DMA1_Stream6 , nSendCount);  //设置数据传输量
		DMA_Cmd(DMA1_Stream6 , ENABLE);               		//DMA使能,开始发送	
		
		while(DMA_GetFlagStatus(DMA1_Stream6,DMA_FLAG_TCIF6) == RESET) ;           // 等待发送完成
		
		DMA_Cmd(DMA1_Stream6 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA1_Stream6) != DISABLE){};	//确保DMA可以被设置
		
        DMA_SetCurrDataCounter(DMA1_Stream6 , 0);
		DMA_ClearFlag(DMA1_Stream6 , DMA_FLAG_TCIF6 | DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 | DMA_FLAG_HTIF6);
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
* 日期：2022-12-24
* 举例：
* 备注：
* 
*   usart3  tx  dma1    数据流3 通道四
	usart3  rx  dma1    数据流1 通道四
*********************************************************************
*********************************************************************/
void usart3_dma_tx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;                                     //初始化结构体变量
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 , ENABLE);	//DMA1 时钟使能				//DMA1时钟使能
	
	DMA_DeInit(DMA1_Stream3);
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE){}; //等待 DMA 可配置						
	DMA_InitStructure.DMA_Channel = DMA_Channel_4; 	//设置 DMA 数据流对应的通道						
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART3_TX_BUF;	    //发送缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					//DMA传输方向：内存--->外设
	DMA_InitStructure.DMA_BufferSize = strlen((const char *)USART3_TX_BUF);		                //数据传输字节数量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					//中等优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  // 禁用fifo
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	DMA_Init(DMA1_Stream3 , &DMA_InitStructure);							//初始化DMA Stream
	DMA_Cmd(DMA1_Stream3 , DISABLE); 										//不开启DMA传输
}
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
* 备注：
* 
*   usart3  tx  dma1    数据流3 通道四
	usart3  rx  dma1    数据流1 通道四
*********************************************************************
*********************************************************************/
void usart3_dma_rx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
 
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 , ENABLE);					//DMA1时钟使能
	DMA_DeInit(DMA1_Stream1);
	while (DMA_GetCmdStatus(DMA1_Stream1) != DISABLE);						//等待DMA可配置  
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  						//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART3->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)USART3_RX_BUF;	    //接收缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;				//DMA传输方向：外设到存储器模式：外设--->内存
	DMA_InitStructure.DMA_BufferSize = USART3_REC_LEN;		                //缓冲大小 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;					    //高优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;  //将 FIFO 阈值设置为 “满” 状态,意味着 DMA 在 FIFO 达到满状态时才会进行数据传输操作。
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	DMA_Init(DMA1_Stream1 , &DMA_InitStructure);							//初始化DMA_Stream5	
	DMA_Cmd(DMA1_Stream1 , ENABLE);  										//开启DMA传输
}
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
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void usart3_dma_tx_send(uint8_t *send_buffer , u32 nSendCount)
{	
	if (nSendCount < USART3_TX_LEN)
	{
		memcpy(USART3_TX_BUF , send_buffer , nSendCount);
		DMA_Cmd(DMA1_Stream3 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);	//确保DMA可以被设置
		DMA_SetCurrDataCounter(DMA1_Stream3 , nSendCount);  //设置数据传输量
		DMA_Cmd(DMA1_Stream3 , ENABLE);               		//DMA使能,开始发送	
		
		while(DMA_GetFlagStatus(DMA1_Stream3,DMA_FLAG_TCIF3) == RESET) ;           // 等待发送完成
		
		DMA_Cmd(DMA1_Stream3 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);	//确保DMA可以被设置
		DMA_SetCurrDataCounter(DMA1_Stream3 , 0);
		DMA_ClearFlag(DMA1_Stream3 , DMA_FLAG_TCIF3 | DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3);
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
* 日期：2022-12-24
* 举例：
* 备注：
* 
* uart4  dam1   pc10   tx   数据流4  通道4
  uart4  dam1   pc11   rx   数据流2  通道4
*********************************************************************
*********************************************************************/
void uart4_dma_tx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;                     //初始化结构体变量
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 , ENABLE);	//DMA1 时钟使能
	
	DMA_DeInit(DMA1_Stream4);
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}; //等待 DMA 可配置						
	DMA_InitStructure.DMA_Channel = DMA_Channel_4; 	//设置 DMA 数据流对应的通道						
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)UART4_TX_BUF;	        //发送缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;					//DMA传输方向：内存--->外设
	DMA_InitStructure.DMA_BufferSize = sizeof((const char *)UART4_TX_BUF);	//数据传输字节数量
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;					//中等优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                  // 禁用fifo
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	DMA_Init(DMA1_Stream4 , &DMA_InitStructure);							//初始化DMA Stream
	DMA_Cmd(DMA1_Stream4 , DISABLE); 										//不开启DMA传输
}
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
* 备注：
* uart4  dam1   pc10   tx   数据流4  通道4
  uart4  dam1   pc11   rx   数据流2  通道4
* 
*********************************************************************
*********************************************************************/
void uart4_dma_rx_configuration(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
 
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 , ENABLE);					//DMA1时钟使能
	DMA_DeInit(DMA1_Stream2);
	while (DMA_GetCmdStatus(DMA1_Stream2) != DISABLE);						//等待DMA可配置  
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  						//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)UART4_RX_BUF;	        //接收缓存指针
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory ;				//DMA传输方向：外设到存储器模式：外设--->内存
	DMA_InitStructure.DMA_BufferSize = UART4_REC_LEN;		                //缓冲大小 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;		//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;					//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;	//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;			//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;							//使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;					    //高优先级 DMA_Priority_High
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;				//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;		//外设突发单次传输
	DMA_Init(DMA1_Stream2 , &DMA_InitStructure);							//初始化DMA_Stream2	
	DMA_Cmd(DMA1_Stream2 , ENABLE);  										//开启DMA传输
}

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
* 备注：
* uart4  dam1   pc10   tx   数据流4  通道4
  uart4  dam1   pc11   rx   数据流2  通道4
* 
*********************************************************************
*********************************************************************/
void uart4_dma_tx_send(uint8_t *send_buffer , u32 nSendCount)
{	
	if (nSendCount < UART4_TX_LEN)
	{
		memcpy(UART4_TX_BUF , send_buffer , nSendCount);
		DMA_Cmd(DMA1_Stream4 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE);	//确保DMA可以被设置
		DMA_SetCurrDataCounter(DMA1_Stream4 , nSendCount);  //设置数据传输量
		DMA_Cmd(DMA1_Stream4 , ENABLE);               		//DMA使能,开始发送	
		
		while(DMA_GetFlagStatus(DMA1_Stream4,DMA_FLAG_TCIF4) == RESET) ;           // 等待发送完成
		
		
//		memset(USART1_TX_BUF,0,strlen((const char*)USART1_TX_BUF));
		DMA_Cmd(DMA1_Stream4 , DISABLE);                    //关闭DMA传输
		while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE);	//确保DMA可以被设置
		DMA_SetCurrDataCounter(DMA1_Stream4 , 0);
		DMA_ClearFlag(DMA1_Stream4 , DMA_FLAG_TCIF4 | DMA_FLAG_FEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_TEIF4 | DMA_FLAG_HTIF4);
	}
}



