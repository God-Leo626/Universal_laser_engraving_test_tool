#ifndef  __USART_RING_BUFF_H__
#define  __USART_RING_BUFF_H__
#include "usart_dma_all.h"
#include "stm32f4xx.h"
#include "stdbool.h"

//**************************************************************************************
#define  RING_BUFFER_LEN   (1*1024)      //  用于保存bin  用于串口3
#define  USART1_RING_BUFFER_LEN   (1024)   //  用于保存bin  用于串口1	
#define  UART4_RING_BUFFER_LEN   (500)    //  用于保存bin  用于串口4

typedef struct ringBuff
{
    unsigned int in;               //写入的位置
    unsigned int out;              //读出的位置
	unsigned int length;           //缓冲区有效数据的长度
    unsigned char buffer[RING_BUFFER_LEN];     //数据域
}stRingBuff;


typedef struct ringBuff_usart1
{
    unsigned int in;               //写入的位置
    unsigned int out;              //读出的位置
	unsigned int length;           //缓冲区有效数据的长度
    unsigned char buffer[USART1_RING_BUFFER_LEN];     //数据域
}stRingBuff_usart1;




//**************************************************************************************
void ring_buffer_init(stRingBuff *ringBuf );
bool is_ring_buffer_full(stRingBuff *ringBuf);
bool is_ringbuffer_empty(stRingBuff *ringBuf);
bool write_onebyte_to_ringbuffer(stRingBuff *ringBuf,char data);
bool read_one_byte_from_ringbuffer(stRingBuff *ringBuf,char *data);

void write_ring_bufer(stRingBuff *ringBuf,char *writeBuf,unsigned int len);
void read_ringbuffer(stRingBuff *ringBuf,char *readBuf,unsigned int len);
unsigned int get_ringbuffer_length(stRingBuff *ringBuf);


//====================================================usart1 
void ring_buffer_init_for_usart1(stRingBuff_usart1 *ringBuf );
bool is_ring_buffer_full_for_usart1(stRingBuff_usart1 *ringBuf);
bool is_ringbuffer_empty_for_usart1(stRingBuff_usart1 *ringBuf);
bool write_onebyte_to_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf,char data);
bool read_one_byte_from_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf,char *data);

void write_ring_bufer_for_usart1(stRingBuff_usart1 *ringBuf,char *writeBuf,unsigned int len);
void read_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf,char *readBuf,unsigned int len);
unsigned int get_ringbuffer_length_for_usart1(stRingBuff_usart1 *ringBuf);
#endif


