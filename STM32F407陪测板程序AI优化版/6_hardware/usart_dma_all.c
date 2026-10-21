#include "stm32f4xx.h"
#include "usart_dma_all.h"
#include "variable_global.h"
#include "usart_ring_buff.h"
#include "stdbool.h"
#include "dma.h"
#include "stdarg.h"
#include "stdio.h"

/* 普通模式 DMA + IDLE 保留原接收方式：停 DMA 后读取稳定 NDTR，
 * 搬入软件队列，再重装 DMA。ISR 不格式化输出、不调用 FreeRTOS API。
 * ORE/帧错误意味着字节流已断裂，通知上层丢弃半帧并等待下一帧边界。
 */
#define USART_DMA_STOP_POLL_LIMIT 100000U

static bool receive_pause(DMA_Stream_TypeDef *stream)
{
    u32 budget = USART_DMA_STOP_POLL_LIMIT;
    DMA_Cmd(stream, DISABLE);
    while (DMA_GetCmdStatus(stream) != DISABLE)
        if (--budget == 0U) return false;
    return true;
}

static void receive_restart(DMA_Stream_TypeDef *stream, uint8_t *buffer,
                            u32 capacity, u32 flags)
{
    DMA_ClearFlag(stream, flags);
    stream->M0AR = (uint32_t)buffer;
    DMA_SetCurrDataCounter(stream, (uint16_t)capacity);
    DMA_Cmd(stream, ENABLE);
}

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
        uint8_t data = (uint8_t)ch;
        (void)stream;
        usart1_dma_tx_send(&data, 1U);
        return ch;
    }	
#else  //使用micro lib 时候编译的代码
	#include <stdio.h>
    int fputc(int ch, FILE *stream)
    {
        uint8_t data = (uint8_t)ch;
        (void)stream;
        usart1_dma_tx_send(&data, 1U);
        return ch;
    }

#endif

#if EN_USART1
/* USART1: PA9 TX / PA10 RX，保持原抢占优先级 3。 */
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
	
	usart1_dma_tx_configuration();  // 串口1 的dma发送  tx 配置
	usart1_dma_rx_configuration();   // 串口1 的dma接收 rx 配置
	ring_buffer_init_for_usart1(&usart1_receive_buffer );        // 初始化环形缓冲队列，用于中断接收保存数据
	//使能uart1（在所有都初始化ok以后，最后使能串口）
	USART_DMACmd(USART1,USART_DMAReq_Tx,ENABLE); //使能串口 1 的 DMA 发送
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE); //使能串口 1 的 DMA 接收
	USART_Cmd(USART1,ENABLE);	//使能串口1 

}

void usart1_printf(char *format, ...)
{
    va_list args;
    va_start(args, format);
    usart_dma_vprintf(USART1, format, args);
    va_end(args);
}

void USART1_IRQHandler(void)
{
    u32 status;
    u32 remaining;
    u32 count;
    bool damaged;
    if (USART_GetITStatus(USART1, USART_IT_IDLE) != SET) return;
    status = USART1->SR;
    if (!receive_pause(DMA2_Stream2))
    {
        (void)USART1->DR;
        ++usart1_receive_buffer.dropped;
        USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);
        return; /* DMA 无法停止时禁止重配活动数据流，避免永久困在 ISR。 */
    }
    status |= USART1->SR;
    (void)USART1->DR;
    remaining = DMA_GetCurrDataCounter(DMA2_Stream2);
    damaged = ((status & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) != 0U) ||
              (DMA_GetFlagStatus(DMA2_Stream2, DMA_FLAG_TEIF2) != RESET) ||
              (DMA_GetFlagStatus(DMA2_Stream2, DMA_FLAG_DMEIF2) != RESET) ||
              remaining > (USART1_REC_LEN);
    count = remaining <= (USART1_REC_LEN) ? (USART1_REC_LEN) - remaining : 0U;
    if (damaged)
    {
        ++usart1_receive_buffer.dropped;
    }
    else if (count > 0U)
    {
        write_ring_bufer_for_usart1(&usart1_receive_buffer, (char *)USART1_RX_BUF, count);
        USART1_REVEIVE_OK = true;
    }
    receive_restart(DMA2_Stream2, USART1_RX_BUF, (USART1_REC_LEN),
                    DMA_FLAG_TCIF2 | DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 |
                    DMA_FLAG_TEIF2 | DMA_FLAG_HTIF2);
}

#endif
#if EN_USART2

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
	
	usart2_dma_tx_configuration();   // 串口2 的dma发送  tx 配置
	usart2_dma_rx_configuration();   // 串口2 的dma接收 rx 配置
	ring_buffer_init_for_usart1(&usart2_receive_buffer );        // 初始化环形缓冲队列，用于中断接收保存数据
	//使能uart2（在所有都初始化ok以后，最后使能串口）
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE); //使能串口 2 的 DMA 发送
	USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); //使能串口 2 的 DMA 接收
	USART_Cmd(USART2,ENABLE);	//使能串口2 
}

void usart2_printf(char *format, ...)
{
    va_list args;
    va_start(args, format);
    usart_dma_vprintf(USART2, format, args);
    va_end(args);
}

void USART2_IRQHandler(void)
{
    u32 status;
    u32 remaining;
    u32 count;
    bool damaged;
    if (USART_GetITStatus(USART2, USART_IT_IDLE) != SET) return;
    status = USART2->SR;
    if (!receive_pause(DMA1_Stream5))
    {
        (void)USART2->DR;
        ++usart2_receive_buffer.dropped;
        USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);
        return; /* DMA 无法停止时禁止重配活动数据流，避免永久困在 ISR。 */
    }
    status |= USART2->SR;
    (void)USART2->DR;
    remaining = DMA_GetCurrDataCounter(DMA1_Stream5);
    damaged = ((status & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) != 0U) ||
              (DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG_TEIF5) != RESET) ||
              (DMA_GetFlagStatus(DMA1_Stream5, DMA_FLAG_DMEIF5) != RESET) ||
              remaining > (USART2_REC_LEN);
    count = remaining <= (USART2_REC_LEN) ? (USART2_REC_LEN) - remaining : 0U;
    if (damaged)
    {
        ++usart2_receive_buffer.dropped;
    }
    else if (count > 0U)
    {
        write_ring_bufer_for_usart1(&usart2_receive_buffer, (char *)USART2_RX_BUF, count);
        USART2_REVEIVE_OK = true;
    }
    receive_restart(DMA1_Stream5, USART2_RX_BUF, (USART2_REC_LEN),
                    DMA_FLAG_TCIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_DMEIF5 |
                    DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5);
}

void usart2_disable(void)
{
    /* 先禁止 IDLE，防止恢复/关闭过程中 ISR 重装 DMA。 */
    USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, DISABLE);
    (void)receive_pause(DMA1_Stream5);
    (void)receive_pause(DMA1_Stream6);
    USART_Cmd(USART2, DISABLE);
    (void)USART2->SR;
    (void)USART2->DR;
    NVIC_ClearPendingIRQ(USART2_IRQn);
}

void usart2_enable(void)
{
    /* 队列和标志必须在开放接收前复位；否则首批回复可能被初始化覆盖。 */
    USART_ITConfig(USART2, USART_IT_IDLE, DISABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, DISABLE);
    if (!receive_pause(DMA1_Stream5)) return;
    ring_buffer_init_for_usart1(&usart2_receive_buffer);
    USART2_REVEIVE_OK = false;
    (void)USART2->SR;
    (void)USART2->DR;
    NVIC_ClearPendingIRQ(USART2_IRQn);
    receive_restart(DMA1_Stream5, USART2_RX_BUF, (USART2_REC_LEN),
                    DMA_FLAG_TCIF5 | DMA_FLAG_FEIF5 | DMA_FLAG_DMEIF5 |
                    DMA_FLAG_TEIF5 | DMA_FLAG_HTIF5);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART2, ENABLE);
}

#endif
#if EN_USART3

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
	ring_buffer_init(&usart3_receive_buffer );    // 初始化环形缓冲队列，用于中断接线保存数据
	
	USART_DMACmd(USART3,USART_DMAReq_Tx,ENABLE); //使能串口 3 的 DMA 发送
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE); //使能串口 3 的 DMA 接收
	//使能uart3（在所有都初始化ok以后，最后使能串口）
	USART_Cmd(USART3,ENABLE);
	// 空读两次，清除串口接收缓冲
	USART_ReceiveData(USART3);
	USART_ReceiveData(USART3);
}

void usart3_printf(char *format, ...)
{
    va_list args;
    va_start(args, format);
    usart_dma_vprintf(USART3, format, args);
    va_end(args);
}

void USART3_IRQHandler(void)
{
    u32 status;
    u32 remaining;
    u32 count;
    bool damaged;
    if (USART_GetITStatus(USART3, USART_IT_IDLE) != SET) return;
    status = USART3->SR;
    if (!receive_pause(DMA1_Stream1))
    {
        (void)USART3->DR;
        ++usart3_receive_buffer.dropped;
        USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);
        return; /* DMA 无法停止时禁止重配活动数据流，避免永久困在 ISR。 */
    }
    status |= USART3->SR;
    (void)USART3->DR;
    remaining = DMA_GetCurrDataCounter(DMA1_Stream1);
    damaged = ((status & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) != 0U) ||
              (DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_TEIF1) != RESET) ||
              (DMA_GetFlagStatus(DMA1_Stream1, DMA_FLAG_DMEIF1) != RESET) ||
              remaining > (USART3_REC_LEN);
    count = remaining <= (USART3_REC_LEN) ? (USART3_REC_LEN) - remaining : 0U;
    if (damaged)
    {
        ++usart3_receive_buffer.dropped;
    }
    else if (count > 0U)
    {
        write_ring_bufer(&usart3_receive_buffer, (char *)USART3_RX_BUF, count);
        USART3_REVEIVE_OK = true;
    }
    receive_restart(DMA1_Stream1, USART3_RX_BUF, (USART3_REC_LEN),
                    DMA_FLAG_TCIF1 | DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 |
                    DMA_FLAG_TEIF1 | DMA_FLAG_HTIF1);
}

#endif
// pc10:   tx     pc11: rx
// UART4 RX: DMA1 Stream2 / Channel4
// UART4 TX: DMA1 Stream4 / Channel4
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
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
    USART_ITConfig(UART4, USART_IT_TXE, DISABLE);
    USART_ITConfig(UART4, USART_IT_IDLE, DISABLE);
    (void)receive_pause(DMA1_Stream2);
    (void)receive_pause(DMA1_Stream4);
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

void uart4_printf(char *format, ...)
{
    va_list args;
    va_start(args, format);
    usart_dma_vprintf(UART4, format, args);
    va_end(args);
}

void UART4_IRQHandler(void)
{
    u32 status;
    u32 remaining;
    u32 count;
    bool damaged;
    if (USART_GetITStatus(UART4, USART_IT_IDLE) != SET) return;
    status = UART4->SR;
    if (!receive_pause(DMA1_Stream2))
    {
        (void)UART4->DR;
        ++uart4_receive_buffer.dropped;
        USART_ITConfig(UART4, USART_IT_IDLE, DISABLE);
        return; /* DMA 无法停止时禁止重配活动数据流，避免永久困在 ISR。 */
    }
    status |= UART4->SR;
    (void)UART4->DR;
    remaining = DMA_GetCurrDataCounter(DMA1_Stream2);
    damaged = ((status & (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) != 0U) ||
              (DMA_GetFlagStatus(DMA1_Stream2, DMA_FLAG_TEIF2) != RESET) ||
              (DMA_GetFlagStatus(DMA1_Stream2, DMA_FLAG_DMEIF2) != RESET) ||
              remaining > (UART4_REC_LEN);
    count = remaining <= (UART4_REC_LEN) ? (UART4_REC_LEN) - remaining : 0U;
    if (damaged)
    {
        ++uart4_receive_buffer.dropped;
    }
    else if (count > 0U)
    {
        write_ring_bufer_for_usart1(&uart4_receive_buffer, (char *)UART4_RX_BUF, count);
        UART4_REVEIVE_OK = true;
    }
    receive_restart(DMA1_Stream2, UART4_RX_BUF, (UART4_REC_LEN),
                    DMA_FLAG_TCIF2 | DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 |
                    DMA_FLAG_TEIF2 | DMA_FLAG_HTIF2);
}

#endif

#if EN_UART5

#endif

