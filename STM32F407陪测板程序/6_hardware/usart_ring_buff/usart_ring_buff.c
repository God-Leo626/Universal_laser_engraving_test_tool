#include "usart_dma_all.h"
#include "usart_ring_buff.h"

#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "stm32f10x.h"
//**************************************************************************************

/*******************************************************************
********************************************************************
* 名称：void ring_buffer_init(stRingBuff *ringBuf )
* 功能： 环形缓冲队列的初始化函数
* 参数：ringBuf ：需要初始化的环形队列
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
void ring_buffer_init(stRingBuff *ringBuf )
{
	ringBuf->in = 0;
	ringBuf->length = 0;
	ringBuf->out = 0;
	memset(ringBuf->buffer,0,sizeof(ringBuf->buffer[RING_BUFFER_LEN]));
}
//  usart1
void ring_buffer_init_for_usart1(stRingBuff_usart1 *ringBuf )
{
	ringBuf->in = 0;
	ringBuf->length = 0;
	ringBuf->out = 0;
	memset(ringBuf->buffer,0,sizeof(ringBuf->buffer[USART1_REC_LEN]));
}
/*******************************************************************
********************************************************************
* 名称：
* 功能： 判断环形队列是否已经满了
* 参数： ringBuf：ring buff的结构体指针 
* 返回值： true ： 满  false ： 没有满
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool is_ring_buffer_full(stRingBuff *ringBuf)
{
	if(ringBuf->length >= RING_BUFFER_LEN)
	{
		printf("the ringbuffer is full\r\n");
		return true;
	}
	return false; 
}

bool is_ring_buffer_full_for_usart1(stRingBuff_usart1 *ringBuf)
{
	if(ringBuf->length >= USART1_RING_BUFFER_LEN)
	{
		printf("the usart1_ringbuffer is full\r\n");
		return true;
	}
	return false; 
}
/*******************************************************************
********************************************************************
* 名称：bool is_ringbuffer_empty(stRingBuff *ringBuf)
* 功能： 判断环形队列是否为空
* 参数： ringBuf ：要判断的环形队列
* 返回值： true :空   false： 不为空，有数据
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool is_ringbuffer_empty(stRingBuff *ringBuf)
{ 
	if(ringBuf->length == 0)
	{
		printf("the ring buffer is empty\r\n");
		return true;
	}
    return false;
}


bool is_ringbuffer_empty_for_usart1(stRingBuff_usart1 *ringBuf)
{ 
	if(ringBuf->length == 0)
	{
		printf("the usart1_ring buffer is empty\r\n");
		return true;
	}
    return false;
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：向环形缓冲队列中写一个字节数据
* 参数： ringBuf:数据写入地址   data ：要写入的一字节数据
* 返回值： false ：满了，写入失败   true ：写入一个字节ok
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool write_onebyte_to_ringbuffer(stRingBuff *ringBuf,char data)
{
	if(is_ring_buffer_full(ringBuf))   //写之前先判断队列是否写满
	{
		return false;
	}
	ringBuf->buffer[ringBuf->in] = data;
	ringBuf->in = (ringBuf->in + 1) % RING_BUFFER_LEN;    //指定下一个要写入的位置，防止越界
	ringBuf->length ++ ;       //有效字符数据 +1 
	return true;
}


bool write_onebyte_to_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf,char data)
{
	if(is_ring_buffer_full_for_usart1(ringBuf))   //写之前先判断队列是否写满
	{
		return false;
	}
	ringBuf->buffer[ringBuf->in] = data;
	ringBuf->in = (ringBuf->in + 1) % USART1_RING_BUFFER_LEN;    //指定下一个要写入的位置，防止越界
	ringBuf->length ++ ;       //有效字符数据 +1 
	return true;
}
/*******************************************************************
********************************************************************
* 名称：char read_one_byte_from_ringbuffer(stRingBuff *ringBuf,char *data)
* 功能： 从环形队列中读一个字节数据
* 参数： ringBuf； 从那个环形队列中读数据   data：读到的数据保存的地方
* 返回值：false :没有读到数据   true : 读到了一个字节数据
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool read_one_byte_from_ringbuffer(stRingBuff *ringBuf,char *data)
{
    if(is_ringbuffer_empty(ringBuf))    //读之前判断队列是否为空
    {
        return false;
    }
    *data = ringBuf->buffer[ringBuf->out];
    ringBuf->out = (ringBuf->out + 1) % RING_BUFFER_LEN;    //指定下一个要读取的位置，防止越界
	ringBuf->length -- ;
    return true;
}

bool read_one_byte_from_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf,char *data)
{
    if(is_ringbuffer_empty_for_usart1(ringBuf))    //读之前判断队列是否为空
    {
        return false;
    }
    *data = ringBuf->buffer[ringBuf->out];
    ringBuf->out = (ringBuf->out + 1) % USART1_RING_BUFFER_LEN;    //指定下一个要读取的位置，防止越界
	ringBuf->length -- ;
    return true;
}
/*******************************************************************
********************************************************************
* 名称：void write_ring_bufer(stRingBuff *ringBuf,char *writeBuf,unsigned int len)
* 功能：将一串字符，按照指定长度一次性写入到环形队列中去
* 参数：ringBuf：被写入的环形队列    writeBuf ：被写入的数据   len：要写入的数据长度
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
void write_ring_bufer(stRingBuff *ringBuf,char *writeBuf,unsigned int len)
{
    unsigned int i;
    for(i = 0; i < len; i++)
    {
        write_onebyte_to_ringbuffer(ringBuf,writeBuf[i]);
    }
}

void write_ring_bufer_for_usart1(stRingBuff_usart1 *ringBuf,char *writeBuf,unsigned int len)
{
    unsigned int i;
    for(i = 0; i < len; i++)
    {
        write_onebyte_to_ringbuffer_for_usart1(ringBuf,writeBuf[i]);
    }
}
/*******************************************************************
********************************************************************
* 名称：void read_ringbuffer(stRingBuff *ringBuf,char *readBuf,unsigned int len)
* 功能：从指定的队列中读出指定长度的数据，并保存到指定位置buffer中
* 参数：ringBuf ：要读的环形队列   readBuf：读出数据保存的地方   len：数据长度
* 返回值： 无
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void read_ringbuffer(stRingBuff *ringBuf,char *readBuf,unsigned int len)
{
    unsigned int i;
    for(i = 0; i < len; i++)
    {
        read_one_byte_from_ringbuffer(ringBuf,&readBuf[i]);
    }
}

void read_ringbuffer_for_usart1(stRingBuff_usart1 *ringBuf,char *readBuf,unsigned int len)
{
    unsigned int i;
    for(i = 0; i < len; i++)
    {
        read_one_byte_from_ringbuffer_for_usart1(ringBuf,&readBuf[i]);
    }
}
/*******************************************************************
********************************************************************
* 名称：int get_ringbuffer_length(stRingBuff *ringBuf)
* 功能：获取已经写入队列的数据长度 有这个方便知道接收完了要从队列中读出多少个数据。
* 参数：ringBuf : 想要获取数据长度的环形队列的地址
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
unsigned int get_ringbuffer_length(stRingBuff *ringBuf)
{
    return  ringBuf->length;
}


unsigned int get_ringbuffer_length_for_usart1(stRingBuff_usart1 *ringBuf)
{
    return  ringBuf->length;
}




