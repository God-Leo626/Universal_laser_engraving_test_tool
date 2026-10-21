#include "usart_dma_all.h"
#include "usart_ring_buff.h"
#include "stdbool.h"
#include "dma.h"
#include "stdarg.h"
#include "string.h" 
#include "my_task.h"
#include "string.h"
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"                               //任务头文件包含
#include "queue.h"                              // 使用消息队列要包含的头文件
#include "semphr.h"                             //信号量头文件包含
#include "systick_delay_freertos.h"
#include "stdio.h"
#include "stm32f10x.h"
#include "time7.h"
#include "time6.h"
#include "time5.h"
//**************************************************************************************
u8 USART1_TX_BUF[USART1_TX_LEN] = {0}; 	        //dma发送缓冲,最大字节
u8 USART1_RX_BUF_1[USART1_REC_LEN] = {0};	    //接收数据缓冲区1
//u8 USART1_RX_BUF_2[USART1_REC_LEN] = {0};	    //接收数据缓冲区2
//bool  USART1_REVEIVE_OK = false ;   	        //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
stRingBuff_usart1  usart1_receive_buffer ;             // 定义一个ringbuffer 的结构体变量,用于串口接收


//u8 USART2_TX_BUF[USART2_TX_LEN] = {0}; 	       //发送缓冲,最大字节
//u8 USART2_RX_BUF_1[USART2_REC_LEN] = {0};	    //接收数据缓冲区1
//u8 USART2_RX_BUF_2[USART2_REC_LEN] = {0};	    //接收数据缓冲区2



u8 USART3_TX_BUF[USART3_TX_LEN] = {0}; 	        //发送缓冲,最大字节
u8 USART3_RX_BUF_1[USART3_REC_LEN] = {0};	    //接收数据缓冲区1
//u8 USART3_RX_BUF_2[USART3_REC_LEN] = {0};	    //接收数据缓冲区2
volatile bool  USART3_REVEIVE_OK = false ;   	        //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
stRingBuff  usart3_receive_buffer ;             // 定义一个ringbuffer 的结构体变量，用于串口接收


u8 UART4_TX_BUF[UART4_TX_LEN] = {0}; 	        //发送缓冲,最大字节
u8 UART4_RX_BUF_1[UART4_REC_LEN] = {0};	        //接收数据缓冲区1
//u8 UART4_RX_BUF_2[UART4_REC_LEN] = {0};	        //接收数据缓冲区2
bool  UART4_REVEIVE_OK = false ;   	        //  收到数据的标志  false ： 没有收到数据   true ： 收到了数据
stRingBuff  uart4_receive_buffer ;             // 定义一个ringbuffer 的结构体变量，用于串口接收


//u8 UART5_TX_BUF[UART5_TX_LEN] = {0}; 	        //发送缓冲,最大字节
//u8 UART5_RX_BUF_1[UART5_REC_LEN] = {0};	        //接收数据缓冲区1
//u8 UART5_RX_BUF_2[UART5_REC_LEN] = {0};	        //接收数据缓冲区2


//bool usart1_witch_buf = false;                  		//标记当前使用的是哪个缓冲区,0：使用USART1_RX_BUF_1；1：使用USART1_RX_BUF_2
//bool usart3_witch_buf = false;                  		//标记当前使用的是哪个缓冲区,0：使用USART3_RX_BUF_1；1：使用USART3_RX_BUF_2
//**************************************************************************************


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
*  printf重定向串口输出 
*********************************************************************
*********************************************************************/
#if NO_USE_MICRO_LIB   // 使用标准库事编译的代码（不使用micro lib）
	#include <stdio.h> 
	/* 告知连接器不从C库链接使用半主机的函数 */
	#pragma import(__use_no_semihosting) 
	/* 定义 _sys_exit() 以避免使用半主机模式 */
	void _sys_exit(int x)
	{
		x = x;
	} 
	/* 标准库需要的支持类型 */
	struct __FILE
	{
		int handle;
	};
	FILE __stdout;
	int fputc(int ch, FILE *stream)
	{
		//堵塞判断串口是否发送完成 
		while((USART1->SR & 0X40) == 0);
		//串口发送完成，将该字符发送 
		USART1->DR = (uint8_t) ch;
		return ch;
	}	
#else  //使用micro lib 时候编译的代码
	#include <stdio.h>
	int fputc(int ch, FILE *stream)
	{
		/* 堵塞判断串口是否发送完成 */
		while((USART1->SR & 0X40) == 0);
		/* 串口发送完成，将该字符发送 */
		USART1->DR = (uint8_t) ch;
		return ch;
	}

#endif
	
	
//**************************************************************************************
#if EN_USART1
/*******************************************************************
********************************************************************
* 名称：void uart1_init(u32 bound)
* 功能：初始化串口1
* 参数：bound： 波特率
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* PA9 :tx     PA10:rx
*********************************************************************
*********************************************************************/
void usart1_init(u32 bound)
{
	//初始化io管脚类型的结构体变量
	GPIO_InitTypeDef  gpio_init_type_struct;
	
	//初始化串口的结构体变量（波特率，校验位，停止位，传输字长）
	USART_InitTypeDef  usart_init_type_strcut;
	
	//初始化中断优先级的结构体变量（抢占优先级，响应优先级）
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	
	//使能时钟 GPIOA,因为UART1 ,用到PA9  PA10 两个管脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);
	//使能uart1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//复位串口1
	USART_DeInit(USART1);
	
	//初始化io口，使其成为TX
	// PA9 = TX   
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_9 ;         //那个管脚
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_50MHz;  //管脚的速率
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_AF_PP ;   //复用推挽输出
	GPIO_Init(GPIOA,&gpio_init_type_struct);
	
	//初始化io口，使其成为RX
	// PA10 = RX
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_10 ;          //那个管脚
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_50MHz;    //管脚的速率
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOA,&gpio_init_type_struct);
	
	//串口参数初始化 115200 8N1  TX/RX  NC
	usart_init_type_strcut.USART_BaudRate = bound; //波特率
	usart_init_type_strcut.USART_HardwareFlowControl =USART_HardwareFlowControl_None; //无硬件数据流控制
	usart_init_type_strcut.USART_Mode = USART_Mode_Rx | USART_Mode_Tx ;                //串口收，发模式
	usart_init_type_strcut.USART_Parity = USART_Parity_No;                             //无校验
	usart_init_type_strcut.USART_StopBits = USART_StopBits_1;                          //停止位1bit
	usart_init_type_strcut.USART_WordLength = USART_WordLength_8b;                     //数据位8bit
	USART_Init(USART1,&usart_init_type_strcut);
	

	
	//设置中断优先级	
	nvic_init_type_strcut.NVIC_IRQChannel = USART1_IRQn ;         //中断通道，具体是哪个外设产生的中断
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority =3;  //抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;	      //响应优先级
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;            //使能通道
	NVIC_Init(&nvic_init_type_strcut);


	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);				  //开启串口空闲中断使能
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);				  //串口接收非空中断使能
	USART_ClearFlag(USART1,USART_FLAG_TC);
	
	dma1_usart1_init();                                           // usart1 dma  接收初始化
	TIM6_Int_Init(150-1,7200-1);                                 //Tout= ((arr+1)*(psc+1))/Tclk；15ms一次中断
	ring_buffer_init_for_usart1(&usart1_receive_buffer );                   // 初始化环形缓冲队列，用于中断接线保存数据
	//使能uart1（在所有都初始化ok以后，最后使能串口）
	USART_Cmd(USART1,ENABLE);	
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
*  usart1---rx---dma-----channel 5
   usart1---tx---dma-----channel 4
* 
*********************************************************************
*********************************************************************/
static void dma1_usart1_tx_data(u8 *buffer,u32 size)  
{
    DMA1_tx_Init.DMA_PeripheralBaseAddr = (u32)&USART1->DR;      // 配置DMA发送时接收源地址
	DMA1_tx_Init.DMA_MemoryBaseAddr = (u32)buffer;               // DMA发送时发射端数组地址
//	DMA1_tx_Init.DMA_BufferSize = strlen((const char*)buffer);   // 发送数据量
	DMA1_tx_Init.DMA_BufferSize = size;                          // 发送数据量
	DMA_Init(DMA1_Channel4, &DMA1_tx_Init);                      // 初始化 usart1 的 tx dma通道
	//****************************************
	DMA_Cmd (DMA1_Channel4,ENABLE);							     // DMA使能,开始发送	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC4) == RESET) ;           // 等待发送完成
	DMA_ClearFlag(DMA1_FLAG_TC4);                                // 清除完成标志
	DMA_Cmd (DMA1_Channel4,DISABLE);							 // DMA失能

}




/*******************************************************************
********************************************************************
* 名称：void usart1_printf(char *format, ...)
* 功能： dma 方式的格式化输出
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void usart1_printf(char *format, ...)
{
	u32 size = 0;
//	int  back_value = 0;   //成功打印到buf中的字符的个数，不包括末尾追加的\0。如果格式化解析失败，则返回负数
	//VA_LIST 是在C语言中解决变参问题的一组宏，所在头文件：#include <stdarg.h>，用于获取不确定个数的参数。
	va_list arg_ptr;	
	va_start(arg_ptr, format); 												            //初始化可变参数列表，设置format为可变长列表的起始点（第一个元素）
	//vsnprintf会自动在写入字符的后面加上停止符\0
//	back_value = vsnprintf((char*)USART1_TX_BUF, USART1_TX_LEN-1, format, arg_ptr);	    // 可接受的最大字符数(非字节数，UNICODE一个字符两个字节), 防止产生数组越界
	                                                                                    //从USART1_TX_BUF的首地址开始拼合，拼合format内容；USART1_TX_LEN-1限制长度，防止产生数组越界
	vsprintf((char*)USART1_TX_BUF,format,arg_ptr);
//	if(back_value < 0)
//	{
//		return ;
//	}
//	else if(back_value >= USART1_TX_LEN  )
//	{
//		back_value = USART1_TX_LEN-1  ;
//	}	
	va_end(arg_ptr);		                        //注意必须关闭
	size = strlen((const char*)USART1_TX_BUF);		//此次发送数据的长度
	dma1_usart1_tx_data(USART1_TX_BUF,size);	            //发送USART1_TX_BUF内容
}
/*******************************************************************
********************************************************************
* 名称：void USART1_IRQHandler(void)
* 功能：串口1中断服务程序 dma 不定长数据接收，并将接收到的数据回显
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
*  usart1  dma   rx   channel 5 
*********************************************************************
*********************************************************************/

void USART1_IRQHandler(void) 
{
//	u8 temp = 0; // 清零空闲中断标志时接收返回值
//	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
//	ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)//接收到数据
	{
		// 接收一个字节并保存到环形队列中
		write_onebyte_to_ringbuffer_for_usart1(&usart1_receive_buffer,USART_ReceiveData(USART1));
		// 等待传输完成
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
		// 关定时器（目的是，在接收到一个字节数据后，打开定时器，在定时器中断服务函数中，加\0结束）
		// 如果有连续不断的数据过来，就会一直关定时器，直到最后一个数据接收完成后，来了一个空闲中断
		// 在空闲中断中，开了定时器，且在开了定时器以后，在定时器定时时间内，没有新的数据在过来，将
		//定时器关闭，就一定会产生定时器中断，在定时器中断服务函数中，将认为此批接收完成，且在最后加
		//如字符结束标识\0，并关闭定时器
		TIM_Cmd(TIM6,DISABLE);           //关闭定时器6
		USART_ClearITPendingBit(USART1,USART_IT_RXNE);      // 清除接收中断标志位
	}
	else if(USART_GetITStatus(USART1,USART_IT_IDLE) == SET) 
	{
		// 打开定时器
		TIM_Cmd(TIM6,ENABLE);           //使能定时器6
		// 清零 串口空闲中断 USART_IT_IDLE标志位
		USART1->SR;
		USART1->DR;
	}
//	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护

}
#endif
//======================================================================================
#if EN_USART2

#endif
//======================================================================================
#if EN_USART3
/*******************************************************************
********************************************************************
* 名称：void uart3_init(u32 bound)
* 功能：将UART3，使用PB10,PB11，复用为串口，115200 8N1 HardwareFlowControl_None
* 参数：u32 bound  波特率
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* PB10  :tx
  PB11 :rx
* 
*********************************************************************
*********************************************************************/
void usart3_init(u32 bound)
{
	//初始化io管脚类型的结构体变量
	GPIO_InitTypeDef  gpio_init_type_struct;
	
	//初始化串口的结构体变量（波特率，校验位，停止位，传输字长）
	USART_InitTypeDef  usart_init_type_strcut;
	
	//初始化中断优先级的结构体变量（抢占优先级，响应优先级）
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	
	//使能时钟 GPIOB,因为UART3 ,用到PB10  PB11 两个管脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB ,ENABLE);
	//使能复用功能时钟
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);
	//使能uart3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);
	//复位串口1
	USART_DeInit(USART3);
	
	
	//初始化io口，使其成为TX
	// PB10 = TX   
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_10 ;         //那个管脚
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_50MHz;  //管脚的速率
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_AF_PP ;   //复用推挽输出
	GPIO_Init(GPIOB,&gpio_init_type_struct);
	
	//初始化io口，使其成为RX
	// PB11 = RX
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_11 ;          //那个管脚
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_50MHz;    //管脚的速率
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOB,&gpio_init_type_struct);
	
	//串口参数初始化 115200 8N1  TX/RX  NC
	usart_init_type_strcut.USART_BaudRate = bound; //波特率
	usart_init_type_strcut.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件流控
	usart_init_type_strcut.USART_Mode = USART_Mode_Rx | USART_Mode_Tx ;                //串口收，发模式
	usart_init_type_strcut.USART_Parity = USART_Parity_No;                             //无校验
	usart_init_type_strcut.USART_StopBits = USART_StopBits_1;                          //停止位1bit
	usart_init_type_strcut.USART_WordLength = USART_WordLength_8b;                     //数据位8bit
	USART_Init(USART3,&usart_init_type_strcut);
	

	
	//设置中断优先级	
	nvic_init_type_strcut.NVIC_IRQChannel = USART3_IRQn ;       //中断通道，具体是哪个外设产生的中断
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;          //使能通道
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority =1; //抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;	      //响应优先级
	NVIC_Init(&nvic_init_type_strcut);

	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);				  //串口空闲中断使能
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);				  //串口接收非空中断使能
	USART_ClearFlag(USART3,USART_FLAG_TC);                        // 清除串口发送完成中断标志

	dma1_usart3_init();                                           // usart3 dma  接收初始化
	
	TIM7_Int_Init(200-1,7200-1);  //Tout= ((arr+1)*(psc+1))/Tclk；15ms一次中断
	ring_buffer_init(&usart3_receive_buffer ); // 初始化环形缓冲队列，用于中断接线保存数据
	
	//使能uart3（在所有都初始化ok以后，最后使能串口）
	USART_Cmd(USART3,ENABLE);
	
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
* usart3  rx   channel 3
   usart3  tx   channel 2
*********************************************************************
*********************************************************************/
static void dma1_usart3_tx_data(u8 *buffer,u32 size)
{
	DMA1_tx3_Init.DMA_PeripheralBaseAddr = (u32)&USART3->DR;      // 配置DMA发送时接收源地址
	DMA1_tx3_Init.DMA_MemoryBaseAddr = (u32)buffer;               // DMA发送时发射端数组地址
//	DMA1_tx3_Init.DMA_BufferSize = strlen((const char*)buffer);   // 发送数据量
	DMA1_tx3_Init.DMA_BufferSize = size;                          // 发送数据量
	DMA_Init(DMA1_Channel2, &DMA1_tx3_Init);                      // 初始化 usart1 的 tx dma通道
	//****************************************
	DMA_Cmd (DMA1_Channel2,ENABLE);							     // DMA使能,开始发送	
	while(DMA_GetFlagStatus(DMA1_FLAG_TC2) == RESET) ;           // 等待发送完成
	DMA_ClearFlag(DMA1_FLAG_TC2);                                // 清除完成标志
	DMA_Cmd (DMA1_Channel2,DISABLE);							 // DMA失能

}

/*******************************************************************
********************************************************************
* 名称：void usart1_printf(char *format, ...)
* 功能： dma 方式的格式化输出
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void usart3_printf(char *format, ...)
{
	u32 size = 0;
	va_list arg_ptr;	          //VA_LIST 是在C语言中解决变参问题的一组宏，所在头文件：#include <stdarg.h>，用于获取不确定个数的参数。
	va_start(arg_ptr, format) ;   //初始化可变参数列表，设置format为可变长列表的起始点（第一个元素）
	                             //从USART1_TX_BUF的首地址开始拼合，拼合format内容；USART1_TX_LEN-1限制长度，防止产生数组越界
	vsprintf((char*)USART3_TX_BUF,format,arg_ptr);	
	va_end(arg_ptr);		                        //注意必须关闭
	size = strlen((const char*)USART3_TX_BUF);		//此次发送数据的长度
	dma1_usart3_tx_data(USART3_TX_BUF,size);	    //发送USART1_TX_BUF内容
}

void u3_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART3_TX_BUF,fmt,ap);
	va_end(ap);
	i = strlen((const char*)USART3_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(USART3,USART3_TX_BUF[j]); 
	} 
}

/*******************************************************************
********************************************************************
* 名称：void USART3_IRQHandler(void)
* 功能：串口3通过环形队列接收数据
		接收数据的逻辑是：串口采用接收缓冲区非空中断接收一个字节
		接收到一个字节后，判断环形缓冲队列非满，将数据保存近环形缓冲
		队列中，关闭定时器7，清除标志位
		在空闲中断中，打开定时器7，清除标志位
		在定时器7中，读取环形缓冲队列里的数据，然后再数据最后加\0结束
		并关闭定时器，置位接收到数据标志，USART3_REVEIVE_OK =true
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void USART3_IRQHandler(void)
{
//	u8 temp = 0; // 清零空闲中断标志时接收返回值
//	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
//	ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收到数据
	{
		
		// 接收一个字节并保存到环形队列中
		write_onebyte_to_ringbuffer(&usart3_receive_buffer,USART_ReceiveData(USART3));
		// 等待传输完成
		while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)== RESET);
		// 关定时器（目的是，在接收到一个字节数据后，打开定时器，在定时器中断服务函数中，加\0结束）
		// 如果有连续不断的数据过来，就会一直关定时器，直到最后一个数据接收完成后，来了一个空闲中断
		// 在空闲中断中，开了定时器，且在开了定时器以后，在定时器定时时间内，没有新的数据在过来，将
		//定时器关闭，就一定会产生定时器中断，在定时器中断服务函数中，将认为此批接收完成，且在最后加
		//如字符结束标识\0，并关闭定时器
		TIM_Cmd(TIM7,DISABLE);           //关闭定时器7
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);      // 清除接收中断标志位
	}
	else if(USART_GetITStatus(USART3,USART_IT_IDLE) == SET) 
	{
		// 打开定时器
		TIM_Cmd(TIM7,ENABLE);           //使能定时器7
		// 清零 串口空闲中断 USART_IT_IDLE标志位
		USART3->SR;
		USART3->DR;
	}
//	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护
}

#endif






//======================================================================================
// pc10:   tx     pc11: rx
// pc11  uart4---rx---dma2-----channel 3
// pc10  uart4---tx---dma2-----channel 5
#if EN_UART4
void uart4_init(u32 bound)
{
	//初始化io管脚类型的结构体变量
	GPIO_InitTypeDef  gpio_init_type_struct;
	
	//初始化串口的结构体变量（波特率，校验位，停止位，传输字长）
	USART_InitTypeDef  usart_init_type_strcut;
	
	//初始化中断优先级的结构体变量（抢占优先级，响应优先级）
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	
	//使能时钟 GPIOc,因为UART4 ,用到Pc10  Pc11 两个管脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC ,ENABLE);
	//使能uart4时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4 , ENABLE);
	
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	//复位串口1
	USART_DeInit(UART4);
	
	//初始化io口，使其成为TX
	// Pc10 = TX   
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_10 ;         //那个管脚
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_50MHz;  //管脚的速率
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_AF_PP ;   //复用推挽输出
	GPIO_Init(GPIOC,&gpio_init_type_struct);
	
	//初始化io口，使其成为RX
	// Pc11 = rX
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_11 ;          //那个管脚
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_50MHz;    //管脚的速率
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOC,&gpio_init_type_struct);
	
	//串口参数初始化  8N1  TX/RX  NC
	usart_init_type_strcut.USART_BaudRate = bound; //波特率
	usart_init_type_strcut.USART_HardwareFlowControl =USART_HardwareFlowControl_None; //无硬件数据流控制
	usart_init_type_strcut.USART_Mode = USART_Mode_Rx | USART_Mode_Tx ;                //串口收，发模式
	usart_init_type_strcut.USART_Parity = USART_Parity_No;                             //无校验
	usart_init_type_strcut.USART_StopBits = USART_StopBits_1;                          //停止位1bit
	usart_init_type_strcut.USART_WordLength = USART_WordLength_8b;                     //数据位8bit
	USART_Init(UART4,&usart_init_type_strcut);
	

	
	//设置中断优先级	
	nvic_init_type_strcut.NVIC_IRQChannel = UART4_IRQn ;         //中断通道，具体是哪个外设产生的中断
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 5;  //抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;	      //响应优先级
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;            //使能通道
	NVIC_Init(&nvic_init_type_strcut);


	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);				  //开启串口空闲中断使能
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);				  //串口接收非空中断使能
	USART_ClearFlag(UART4,USART_FLAG_TC);
	
	dma2_uart4_init();                                           // uart4 dma  接收初始化
	TIM5_Int_Init(150-1,7200-1);                                 //Tout= ((arr+1)*(psc+1))/Tclk；15ms一次中断
	ring_buffer_init(&uart4_receive_buffer );                   // 初始化环形缓冲队列，用于中断接线保存数据
	//使能uart1（在所有都初始化ok以后，最后使能串口）
	USART_Cmd(UART4,ENABLE);	
}


// pc11  uart4---rx---dma2-----channel 3
// pc10  uart4---tx---dma2-----channel 5
static void dma2_uart4_tx_data(u8 *buffer,u32 size)  
{
    DMA2_tx4_Init.DMA_PeripheralBaseAddr = (u32)&UART4->DR;      // 配置DMA发送时接收源地址
	DMA2_tx4_Init.DMA_MemoryBaseAddr = (u32)buffer;               // DMA发送时发射端数组地址
//	DMA1_tx_Init.DMA_BufferSize = strlen((const char*)buffer);   // 发送数据量
	DMA2_tx4_Init.DMA_BufferSize = size;                          // 发送数据量
	DMA_Init(DMA2_Channel5, &DMA2_tx4_Init);                      // 初始化 uart4 的 tx dma通道
	//****************************************
	DMA_Cmd (DMA2_Channel5,ENABLE);							     // DMA使能,开始发送	
	while(DMA_GetFlagStatus(DMA2_FLAG_TC5) == RESET) ;           // 等待发送完成
	DMA_ClearFlag(DMA2_FLAG_TC5);                                // 清除完成标志
	DMA_Cmd (DMA2_Channel5,DISABLE);							 // DMA失能
//	memset(buffer,0,strlen((char*)buffer));                      // 清空发送缓冲区
}


// pc11  uart4---rx---dma2-----channel 3
// pc10  uart4---tx---dma2-----channel 5
void uart4_printf(char *format, ...)
{
	u32 size = 0;
//	int  back_value = 0;   //成功打印到buf中的字符的个数，不包括末尾追加的\0。如果格式化解析失败，则返回负数
	//VA_LIST 是在C语言中解决变参问题的一组宏，所在头文件：#include <stdarg.h>，用于获取不确定个数的参数。
	va_list arg_ptr;	
	va_start(arg_ptr, format); 												            //初始化可变参数列表，设置format为可变长列表的起始点（第一个元素）
	//vsnprintf会自动在写入字符的后面加上停止符\0
//	back_value = vsnprintf((char*)USART1_TX_BUF, USART1_TX_LEN-1, format, arg_ptr);	    // 可接受的最大字符数(非字节数，UNICODE一个字符两个字节), 防止产生数组越界
	                                                                                    //从UART4_TX_BUF的首地址开始拼合，拼合format内容；USART1_TX_LEN-1限制长度，防止产生数组越界
	vsprintf((char*)UART4_TX_BUF,format,arg_ptr);
	
	va_end(arg_ptr);		                        //注意必须关闭
	size = strlen((const char*)UART4_TX_BUF);		//此次发送数据的长度
	dma2_uart4_tx_data(UART4_TX_BUF,size);	            //发送UART4_TX_BUF内容
}


void UART4_IRQHandler(void) 
{
//	u8 temp = 0; // 清零空闲中断标志时接收返回值
//	uint32_t ulReturn; // 保存临界代码段保护函数的返回值
//	ulReturn = taskENTER_CRITICAL_FROM_ISR();  // 进入临界代码段
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)//接收到数据
	{
		// 接收一个字节并保存到环形队列中
		write_onebyte_to_ringbuffer(&uart4_receive_buffer,USART_ReceiveData(UART4));
		// 等待传输完成
		while(USART_GetFlagStatus(UART4,USART_FLAG_TXE) == RESET);
		// 关定时器（目的是，在接收到一个字节数据后，打开定时器，在定时器中断服务函数中，加\0结束）
		// 如果有连续不断的数据过来，就会一直关定时器，直到最后一个数据接收完成后，来了一个空闲中断
		// 在空闲中断中，开了定时器，且在开了定时器以后，在定时器定时时间内，没有新的数据在过来，将
		//定时器关闭，就一定会产生定时器中断，在定时器中断服务函数中，将认为此批接收完成，且在最后加
		//如字符结束标识\0，并关闭定时器
		TIM_Cmd(TIM5,DISABLE);           //关闭定时器6
		USART_ClearITPendingBit(UART4,USART_IT_RXNE);      // 清除接收中断标志位
	}
	else if(USART_GetITStatus(UART4,USART_IT_IDLE) == SET) // 空闲中断 
	{
		// 打开定时器
		TIM_Cmd(TIM5,ENABLE);           //使能定时器5
		// 清零 串口空闲中断 USART_IT_IDLE标志位
		UART4->SR;
		UART4->DR;
	}
//	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护

}
#endif

//======================================================================================

#if EN_UART5

#endif

