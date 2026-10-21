#include "stm32f4xx.h"
#include "usart_dma_all.h"
#include "variable_global.h"
#include "usart_ring_buff.h"
#include "stdbool.h"
#include "dma.h"
#include "stdarg.h"
//#include "my_task.h"
#include "string.h"
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"                               //任务头文件包含
#include "queue.h"                              // 使用消息队列要包含的头文件
#include "semphr.h"                             //信号量头文件包含
#include "systick_delay_freertos.h"
#include "stdio.h"
#include "time7.h"
#include "time6.h"
#include "time5.h"

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
#if NO_USE_MICRO_LIB   // 使用标准库时编译的代码（不使用micro lib）
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	//使能uart1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 , ENABLE);
	
	//复位串口1
	USART_DeInit(USART1);
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);  //GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); //GPIOA10复用为USART1
 
	
	
	//初始化io口，使其成为TX  RX
	// PA9 = TX    PA10 = RX
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_AF;           //复用功能
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_100MHz;	   //速度50MHz
	gpio_init_type_struct.GPIO_OType = GPIO_OType_PP;         //推挽复用输出
	gpio_init_type_struct.GPIO_PuPd = GPIO_PuPd_UP;           //上拉
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

	//使能串口相关中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);				  //开启串口空闲中断使能
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);				  //串口接收非空中断关闭
	USART_ITConfig(USART1, USART_IT_TXE, DISABLE);				  //禁止USART1发送空中断
//	USART_ClearFlag(USART1,USART_FLAG_TC);
	
	usart1_dma_tx_configuration();  // 串口1 的dma发送  tx 配置
	usart1_dma_rx_configuration();   // 串口1 的dma接收 rx 配置
//	TIM6_Int_Init(100-1,16800-1);                                 //Tout= ((arr+1)*(psc+1))/Tclk；10ms一次中断
	ring_buffer_init_for_usart1(&usart1_receive_buffer );        // 初始化环形缓冲队列，用于中断接收保存数据
	//使能uart1（在所有都初始化ok以后，最后使能串口）
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口 1 的 DMA 发送
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); //使能串口 1 的 DMA 接收
	USART_Cmd(USART1,ENABLE);	//使能串口1 
    
    
    
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
* usart1  dma2   tx  数据流7  通道4
                 rx  数据流2  通道4
*********************************************************************
*********************************************************************/
void usart1_printf(char *format, ...)
{
//    char print_buf[512] = {0};
	u32 size = 0;
	va_list arg_ptr;                                // VA_LIST 是在C语言中解决变参问题的一组宏，所在头文件：#include <stdarg.h>，用于获取不确定个数的参数。	
	va_start(arg_ptr, format);                  	// 初始化可变参数列表，设置format为可变长列表的起始点（第一个元素）
	vsprintf((char*)USART1_TX_BUF,format,arg_ptr);
	va_end(arg_ptr);		                        // 注意必须关闭
	size = strlen((const char*)USART1_TX_BUF);		    // 此次发送数据的长度
	usart1_dma_tx_send((uint8_t*)USART1_TX_BUF , size); // 发送USART1_TX_BUF内容
	
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
*  usart1  dma2   tx  数据流7  通道4
                  rx  数据流2  通道4
*********************************************************************
*********************************************************************/

void USART1_IRQHandler(void) 
{
    
	u32 receive_data_number = 0;
//	u8 read_buf[USART1_REC_LEN] = {0};
    

	if(USART_GetITStatus(USART1,USART_IT_IDLE) == SET)  // 空闲中断处理
    {
        DMA_Cmd(DMA2_Stream2, DISABLE);
        while(DMA_GetCmdStatus(DMA2_Stream2) != DISABLE){};
		// 清零 串口空闲中断 USART_IT_IDLE标志位
		USART1->SR;
		USART1->DR;
        
		DMA_ClearFlag(DMA2_Stream2 , DMA_FLAG_TCIF2 | DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 | DMA_FLAG_HTIF2);
        
		receive_data_number =  USART1_REC_LEN - DMA_GetCurrDataCounter(DMA2_Stream2);  // dma 收到的数据长度
		if(receive_data_number > 0) // dma的数据长度为有效值，表示有收到数据
		{
//            usart1_printf("ISR:%d,[%02X,%02X]\r\n", receive_data_number, USART1_RX_BUF[0], USART1_RX_BUF[1]);
			write_ring_bufer_for_usart1(&usart1_receive_buffer,(char*)USART1_RX_BUF,receive_data_number); //将dma接收到且保存在USART1_RX_BUF中的数据保存近usart1_receive_buffer中
//			write_onebyte_to_ringbuffer_for_usart1(&usart1_receive_buffer,'\0');             // 写入字符串结束标识
			USART1_REVEIVE_OK = true;
            
//			read_ringbuffer_for_usart1(&usart1_receive_buffer,(char*)read_buf,receive_data_number);
//			usart1_printf("buf=%s",read_buf);
		}
//		memset(USART1_RX_BUF,0,USART1_REC_LEN); // 清理全局变量USART1_RX_BUF，以备下次使用
//		while (DMA_GetCmdStatus(DMA2_Stream2) != DISABLE){};	//确保DMA可以被设置
        DMA2_Stream2->M0AR = (uint32_t)USART1_RX_BUF;  // 在重新使能 DMA 之前，重置内存地址指针到缓冲区起始位置
		DMA_SetCurrDataCounter(DMA2_Stream2 , USART1_REC_LEN);
		DMA_Cmd(DMA2_Stream2 , ENABLE);  // 打开dam接收

    }
//	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护



}



#endif



//======================================================================================
#if EN_USART2
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
*   USART2_TX	DMA1	Stream6	Channel4
    USART2_RX	DMA1	Stream5	Channel4   
*    PA2-----TX
     PA3-----RX
*********************************************************************
*********************************************************************/

void usart2_init(u32 bound)
{
	//初始化io管脚类型的结构体变量
	GPIO_InitTypeDef  gpio_init_type_struct;
	
	//初始化串口的结构体变量（波特率，校验位，停止位，传输字长）
	USART_InitTypeDef  usart_init_type_strcut;
	
	//初始化中断优先级的结构体变量（抢占优先级，响应优先级）
	NVIC_InitTypeDef  nvic_init_type_strcut;
	
	
	//使能时钟 GPIOA,因为UART2 ,用到PA2  PA3 两个管脚
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	//使能uart2时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2 , ENABLE);
	
	//复位串口2
	USART_DeInit(USART2);
	//串口2对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);  //GPIOA2复用为USART2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);  //GPIOA3复用为USART2
 
	
	
	//初始化io口，使其成为TX  RX
	// PA2 = TX    PA3 = RX
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //GPIOA2与GPIOA3
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_AF;           //复用功能
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_100MHz;	   //速度50MHz
	gpio_init_type_struct.GPIO_OType = GPIO_OType_PP;         //推挽复用输出
	gpio_init_type_struct.GPIO_PuPd = GPIO_PuPd_UP;           //上拉
	GPIO_Init(GPIOA,&gpio_init_type_struct);
	
	
	//串口参数初始化 115200 8N1  TX/RX  NC
	usart_init_type_strcut.USART_BaudRate = bound; //波特率
	usart_init_type_strcut.USART_HardwareFlowControl =USART_HardwareFlowControl_None; //无硬件数据流控制
	usart_init_type_strcut.USART_Mode = USART_Mode_Rx | USART_Mode_Tx ;                //串口收，发模式
	usart_init_type_strcut.USART_Parity = USART_Parity_No;                             //无校验
	usart_init_type_strcut.USART_StopBits = USART_StopBits_1;                          //停止位1bit
	usart_init_type_strcut.USART_WordLength = USART_WordLength_8b;                     //数据位8bit
	USART_Init(USART2,&usart_init_type_strcut);
	
	//设置中断优先级	
	nvic_init_type_strcut.NVIC_IRQChannel = USART2_IRQn ;         //中断通道，具体是哪个外设产生的中断
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority =2;  //抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;	      //响应优先级
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;            //使能通道
	NVIC_Init(&nvic_init_type_strcut);

	//使能串口相关中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);				  //开启串口空闲中断使能
	USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);				  //串口接收非空中断关闭
	USART_ITConfig(USART2, USART_IT_TXE, DISABLE);				  //禁止USART1发送空中断
//	USART_ClearFlag(USART1,USART_FLAG_TC);
	
	usart2_dma_tx_configuration();   // 串口2 的dma发送  tx 配置
	usart2_dma_rx_configuration();   // 串口2 的dma接收 rx 配置
//	TIM6_Int_Init(100-1,16800-1);                                 //Tout= ((arr+1)*(psc+1))/Tclk；10ms一次中断
	ring_buffer_init_for_usart1(&usart2_receive_buffer );        // 初始化环形缓冲队列，用于中断接收保存数据
	//使能uart2（在所有都初始化ok以后，最后使能串口）
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE); //使能串口 2 的 DMA 发送
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); //使能串口 2 的 DMA 接收
	USART_Cmd(USART2,ENABLE);	//使能串口2 
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
*    USART2_TX	DMA1	Stream6	Channel4
     USART2_RX	DMA1	Stream5	Channel4   
*    PA2-----TX
     PA3-----RX
* 
*********************************************************************
*********************************************************************/
void usart2_printf(char *format, ...)
{
//    char print_buf[512] = {0};
	u32 size = 0;
	va_list arg_ptr;             //VA_LIST 是在C语言中解决变参问题的一组宏，所在头文件：#include <stdarg.h>，用于获取不确定个数的参数。	
	va_start(arg_ptr, format); 	//初始化可变参数列表，设置format为可变长列表的起始点（第一个元素）
	vsprintf((char*)USART2_TX_BUF,format,arg_ptr);
	va_end(arg_ptr);		                        //注意必须关闭
	size = strlen((const char*)USART2_TX_BUF);		//此次发送数据的长度
	usart2_dma_tx_send((uint8_t*)USART2_TX_BUF , size);       //发送USART2_TX_BUF内容
	
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
*    USART2_TX	DMA1	Stream6	Channel4
     USART2_RX	DMA1	Stream5	Channel4   
*    PA2-----TX
     PA3-----RX
* 
*********************************************************************
*********************************************************************/
void USART2_IRQHandler(void) 
{
	u32 receive_data_number = 0;
//	u8 read_buf[USART1_REC_LEN] = {0};
	if(USART_GetITStatus(USART2,USART_IT_IDLE) == SET)  // 空闲中断处理
    {
        DMA_Cmd(DMA1_Stream5 , DISABLE);  // 关闭dma
        while (DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);	//确保DMA可以被设置
		
		USART2->SR;  // 清零 串口空闲中断 USART_IT_IDLE标志位
		USART2->DR;
		DMA_ClearFlag(DMA1_Stream5 , DMA_FLAG_TCIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5);
        
		receive_data_number =  USART2_REC_LEN - DMA_GetCurrDataCounter(DMA1_Stream5);  // dma 收到的数据长度
		if(receive_data_number > 0) // dma的数据长度为有效值，表示有收到数据
		{
			write_ring_bufer_for_usart1(&usart2_receive_buffer,(char*)USART2_RX_BUF,receive_data_number); //将dma接收到且保存在USART2_RX_BUF中的数据保存近usart2_receive_buffer中
//			write_onebyte_to_ringbuffer_for_usart1(&usart2_receive_buffer,'\0');             // 写入字符串结束标识
			USART2_REVEIVE_OK = true;

		}
//		memset(USART2_RX_BUF,0,sizeof(USART2_RX_BUF)); // 清理全局变量USART2_RX_BUF，以备下次使用
		DMA1_Stream5->M0AR = (uint32_t)USART2_RX_BUF;  // 在重新使能 DMA 之前，重置内存地址指针到缓冲区起始位置
		DMA_SetCurrDataCounter(DMA1_Stream5 , USART2_REC_LEN);
		DMA_Cmd(DMA1_Stream5 , ENABLE);  // 打开dam接收

    }
//	taskEXIT_CRITICAL_FROM_ISR( ulReturn );                            // 退出临界代码段保护

}

/*******************************************************************
********************************************************************
* 名称：
* 功能：关闭串口2（DMA接收 + 中断）
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void usart2_disable(void)
{
    USART_Cmd(USART2, DISABLE);                       // 关闭串口外设
    USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);   // 关闭串口DMA接收请求
    USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);   // 关闭串口DMA发送请求
    DMA_Cmd(DMA1_Stream5, DISABLE);                   // 关闭RX DMA
    while (DMA_GetCmdStatus(DMA1_Stream5) != DISABLE);
    USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);   // 关闭空闲中断
}

/*******************************************************************
********************************************************************
* 名称：
* 功能： 打开串口2（恢复DMA接收，无需重新初始化）
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void usart2_enable(void)
{
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);    // 重新使能空闲中断

    DMA1_Stream5->M0AR = (uint32_t)USART2_RX_BUF;     // 重置DMA内存地址
    DMA_ClearFlag(DMA1_Stream5, DMA_FLAG_TCIF5 | DMA_FLAG_FEIF5 |
                  DMA_FLAG_DMEIF5 | DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5);
    DMA_SetCurrDataCounter(DMA1_Stream5, USART2_REC_LEN);
    DMA_Cmd(DMA1_Stream5, ENABLE);                    // 使能RX DMA

    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);    // 使能串口DMA接收请求
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);    // 使能串口DMA发送请求
    USART_Cmd(USART2, ENABLE);                        // 使能串口外设

    ring_buffer_init_for_usart1(&usart2_receive_buffer); // 清空环形缓冲区
    USART2_REVEIVE_OK = false;                        // 清除接收标志
}

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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	//使能uart3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3 , ENABLE);
	
	//复位串口3的设置
	USART_DeInit(USART3);
	//串口3对应引脚复用映射
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);  //GPIO PB10 复用为USART3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);  //GPIO PB11 复用为USART3
	
	
	//初始化io口，使其成为TX  RX
	// PB10 = TX    PB11 = RX
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOB10 与GPIOB11
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_AF;             //复用功能
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_100MHz;	    //速度50MHz
	gpio_init_type_struct.GPIO_OType = GPIO_OType_PP;           //推挽复用输出
	gpio_init_type_struct.GPIO_PuPd = GPIO_PuPd_UP;             //上拉
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
	nvic_init_type_strcut.NVIC_IRQChannel = USART3_IRQn ;         //中断通道，具体是哪个外设产生的中断
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;            //使能通道
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 1;  //抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;	      //响应优先级
	NVIC_Init(&nvic_init_type_strcut);

	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);				  // 串口空闲中断使能
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);				  // 串口接收非空中断禁能
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE);				  //禁止USART3发送空中断
	USART_ClearFlag(USART3,USART_FLAG_TC);                        // 清除串口发送完成中断标志

	usart3_dma_tx_configuration();   // 串口3 的dma发送  tx 配置
	usart3_dma_rx_configuration();   // 串口3 的dma接收  rx 配置
	
//	TIM7_Int_Init(150-1,16800-1);  //Tout= ((arr+1)*(psc+1))/Tclk；10ms一次中断
	ring_buffer_init(&usart3_receive_buffer );    // 初始化环形缓冲队列，用于中断接线保存数据
	
	USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE); //使能串口 3 的 DMA 发送
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); //使能串口 3 的 DMA 接收
	//使能uart3（在所有都初始化ok以后，最后使能串口）
	USART_Cmd(USART3,ENABLE);
	// 空读两次，清除串口接收缓冲
	USART_ReceiveData(USART3);
	USART_ReceiveData(USART3);
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
	usart3_dma_tx_send(USART3_TX_BUF,size);	    //发送USART1_TX_BUF内容
}

//void u3_printf(char* fmt,...)  
//{  
//	u16 i,j; 
//	va_list ap; 
//	va_start(ap,fmt);
//	vsprintf((char*)USART3_TX_BUF,fmt,ap);
//	va_end(ap);
//	i = strlen((const char*)USART3_TX_BUF);		//此次发送数据的长度
//	for(j=0;j<i;j++)							//循环发送数据
//	{
//		while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
//		USART_SendData(USART3,USART3_TX_BUF[j]); 
//	} 
//}

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
*  usart3  tx  dma1    数据流3 通道四
	usart3  rx  dma1    数据流1 通道四
* 
*********************************************************************
*********************************************************************/
void USART3_IRQHandler(void)
{
	u32 receive_data_number = 0;
	if(USART_GetITStatus(USART3,USART_IT_IDLE) == SET)       // 空闲中断触发
	{
		// 清零 串口空闲中断 USART_IT_IDLE标志位
		USART3->SR;
		USART3->DR;
		
		DMA_Cmd(DMA1_Stream1 , DISABLE);  // 关闭dma
		DMA_ClearFlag(DMA1_Stream1 , DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);
		receive_data_number =  USART3_REC_LEN - DMA_GetCurrDataCounter(DMA1_Stream1);  // dma 收到的数据长度
//		usart1_printf("receive_data_number = %u\r\n",receive_data_number);
		if(receive_data_number > 0)      // dma的数据长度为有效值，表示有收到数据
		{
			write_ring_bufer(&usart3_receive_buffer,(char*)USART3_RX_BUF,receive_data_number); //将dma接收到且保存在USART1_RX_BUF中的数据保存进usart3_receive_buffer中
			
			USART3_REVEIVE_OK = true ;
//			read_ringbuffer(&usart3_receive_buffer,(char*)read_buf,receive_data_number);
//			usart3_printf("%s",read_buf);
		}
		memset(USART3_RX_BUF,0,sizeof(USART3_RX_BUF));
		while (DMA_GetCmdStatus(DMA1_Stream1) != DISABLE);	//确保DMA可以被设置
		DMA_SetCurrDataCounter(DMA1_Stream1 , USART3_REC_LEN);  //重新设置传输数据长度
		DMA_Cmd(DMA1_Stream1 , ENABLE);  // 打开dma接收
		// USART3_REVEIVE_OK = true;   	                                      // 收到数据的标志  false ： 没有收到数据   true ： 收到了数据
		
		
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
	
	
	//使能时钟 GPIOC,因为UART4 ,用到PC10  PC11 两个管脚
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	//使能uart4时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4 , ENABLE);
	//复位串口4的设置
	USART_DeInit(UART4);
	
	//串口4对应引脚复用映射
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_UART4);  //GPIO PC10 复用为UART4
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_UART4);  //GPIO PC11 复用为UART4
	
	
	//初始化io口，使其成为TX  RX
	// PC10 = TX    PC11 = RX
	gpio_init_type_struct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOC10 与GPIOC11
	gpio_init_type_struct.GPIO_Mode = GPIO_Mode_AF;           //复用功能
	gpio_init_type_struct.GPIO_Speed = GPIO_Speed_100MHz;	   //速度50MHz
	gpio_init_type_struct.GPIO_OType = GPIO_OType_PP;         //推挽复用输出
	gpio_init_type_struct.GPIO_PuPd = GPIO_PuPd_UP;           //上拉
	GPIO_Init(GPIOC,&gpio_init_type_struct);
	
	//串口参数初始化 115200 8N1  TX/RX  NC
	usart_init_type_strcut.USART_BaudRate = bound; //波特率
	usart_init_type_strcut.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //无硬件流控
	usart_init_type_strcut.USART_Mode = USART_Mode_Rx | USART_Mode_Tx ;                //串口收，发模式
	usart_init_type_strcut.USART_Parity = USART_Parity_No;                             //无校验
	usart_init_type_strcut.USART_StopBits = USART_StopBits_1;                          //停止位1bit
	usart_init_type_strcut.USART_WordLength = USART_WordLength_8b;                     //数据位8bit
	USART_Init(UART4,&usart_init_type_strcut);
	

	
	//设置中断优先级	
	nvic_init_type_strcut.NVIC_IRQChannel = UART4_IRQn ;          //中断通道，具体是哪个外设产生的中断
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 2;  //抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;	      //响应优先级
	nvic_init_type_strcut.NVIC_IRQChannelCmd = ENABLE;            //使能通道
	NVIC_Init(&nvic_init_type_strcut);

	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);				  // 串口空闲中断使能
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);				  // 串口接收非空中断禁能
	USART_ITConfig(UART4, USART_IT_TXE, DISABLE);				  // 禁止 发送空中断
	USART_ClearFlag(UART4,USART_FLAG_TC);                         // 清除串口发送完成中断标志

	uart4_dma_tx_configuration();   // 串口4 的dma发送  tx 配置
	uart4_dma_rx_configuration();   // 串口4 的dma接收  rx 配置
	

	ring_buffer_init_for_usart1(&uart4_receive_buffer );    // 初始化环形缓冲队列，用于中断接线保存数据
	
	USART_DMACmd(UART4,USART_DMAReq_Tx,ENABLE); //使能串口 4 的 DMA 发送
	USART_DMACmd(UART4,USART_DMAReq_Rx,ENABLE); //使能串口 4 的 DMA 接收
	//使能uart4（在所有都初始化ok以后，最后使能串口）
	USART_Cmd(UART4,ENABLE);
	// 空读两次，清除串口接收缓冲
	USART_ReceiveData(UART4);
	USART_ReceiveData(UART4);
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void uart4_close(void)
{
	//初始化中断优先级的结构体变量（抢占优先级，响应优先级）
	NVIC_InitTypeDef  nvic_init_type_strcut;

	//设置中断优先级	
	nvic_init_type_strcut.NVIC_IRQChannel = UART4_IRQn ;       //中断通道，具体是哪个外设产生的中断
	nvic_init_type_strcut.NVIC_IRQChannelPreemptionPriority = 2; //抢占优先级
	nvic_init_type_strcut.NVIC_IRQChannelSubPriority = 0;	      //响应优先级
	nvic_init_type_strcut.NVIC_IRQChannelCmd = DISABLE;          //禁能通道
	NVIC_Init(&nvic_init_type_strcut);

	// 关闭发送和接收中断
	USART_ITConfig(UART4, USART_IT_RXNE|USART_IT_TXE, DISABLE);
	USART_Cmd(UART4,DISABLE);									
	
	// 禁止串口的dma 发送和接收
	USART_DMACmd(UART4,USART_DMAReq_Tx,DISABLE); //禁能串口 4 的 DMA 发送
	USART_DMACmd(UART4,USART_DMAReq_Rx,DISABLE); //禁能串口 4 的 DMA 接收
	
	// 空读两次，清除串口接收缓冲
	USART_ReceiveData(UART4);
	USART_ReceiveData(UART4);
	// 将USARTx外围寄存器反初始化为它们的默认重置值。
	USART_DeInit( UART4 );							

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
void uart4_printf(char *format, ...)
{
	u32 size = 0;
	va_list arg_ptr;	          //VA_LIST 是在C语言中解决变参问题的一组宏，所在头文件：#include <stdarg.h>，用于获取不确定个数的参数。
	va_start(arg_ptr, format) ;   //初始化可变参数列表，设置format为可变长列表的起始点（第一个元素）
	                             //从UART4_TX_BUF的首地址开始拼合，拼合format内容；UART4_TX_LEN-1限制长度，防止产生数组越界
	vsprintf((char*)UART4_TX_BUF,format,arg_ptr);	
	va_end(arg_ptr);		                        //注意必须关闭
	size = strlen((const char*)UART4_TX_BUF);		//此次发送数据的长度
	uart4_dma_tx_send(UART4_TX_BUF,size);	        //发送UART4_TX_BUF内容
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
void UART4_IRQHandler(void) 
{
	u32 receive_data_number = 0;
	if(USART_GetITStatus(UART4,USART_IT_IDLE) == SET)       // 空闲中断触发
	{
		// 清零 串口空闲中断 USART_IT_IDLE标志位
		UART4->SR;
		UART4->DR;
		
		DMA_Cmd(DMA1_Stream2 , DISABLE);  // 关闭dma
		DMA_ClearFlag(DMA1_Stream2 , DMA_FLAG_TCIF2 | DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 | DMA_FLAG_HTIF2);
		receive_data_number =  UART4_REC_LEN - DMA_GetCurrDataCounter(DMA1_Stream2);  // dma 收到的数据长度
		if(receive_data_number > 0)      // dma的数据长度为有效值，表示有收到数据
		{
			write_ring_bufer_for_usart1(&uart4_receive_buffer,(char*)UART4_RX_BUF,receive_data_number); //将dma接收到且保存在UART4_RX_BUF中的数据保存进uart4_receive_buffer中
//			uart4_printf("%s",UART4_RX_BUF);
			UART4_REVEIVE_OK = true;   	  // 收到数据的标志  false ： 没有收到数据   true ： 收到了数据
		}
		memset(UART4_RX_BUF,0,sizeof(UART4_RX_BUF)); // 清理全局数组，以备下次在接收新数据(要在if函数体外清理)
		while (DMA_GetCmdStatus(DMA1_Stream2) != DISABLE);	//确保DMA可以被设置
		DMA_SetCurrDataCounter(DMA1_Stream2 , UART4_REC_LEN);  //重新设置传输数据长度
		DMA_Cmd(DMA1_Stream2 , ENABLE);  // 打开dma接收
		
	}

}

#endif

//======================================================================================

#if EN_UART5

#endif

