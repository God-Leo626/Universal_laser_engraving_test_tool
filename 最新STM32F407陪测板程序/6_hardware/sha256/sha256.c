#include "sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "usart_dma_all.h"
#include "usart_ring_buff.h"
//**************************************************************************************
volatile uint64_t  total_len = 0;                // 保存此批要进行sha256校验的数据的总大小
volatile uint64_t  remain_num = 0;               // 保存剩下要处理的数据大小
volatile unsigned char hash_buf[64] = {0};       // 保存sha256 校验的值 64字符

volatile stRingBuff  sha256_data_storage ;       // 环形队列结构体变量，用于保存要校验的数据（注意buf大小）
volatile bool update_onetime = true ;            // 在sha256_data_stream_update中，用来保证第一次进入执行

//SHA256_CTX  ctx;                      // 辅助校验的结构体
//**************************************************************************************
void SHA256_CTX_init(SHA256_CTX  p_ctx)
{
	memset(p_ctx.data,0,64);
	p_ctx.datalen = 0; 
	p_ctx.bitlen = 0;
	memset(p_ctx.state,0,sizeof(p_ctx.state));
}

static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2

};
//第一个块的输入是预设好的hash初值
static void sha256_init(SHA256_CTX *ctx)
{
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667;
	ctx->state[1] = 0xbb67ae85;
	ctx->state[2] = 0x3c6ef372;
	ctx->state[3] = 0xa54ff53a;
	ctx->state[4] = 0x510e527f;
	ctx->state[5] = 0x9b05688c;
	ctx->state[6] = 0x1f83d9ab;
	ctx->state[7] = 0x5be0cd19;
}
//一个循环加密的过程，不断将原始信息进行打乱
static void sha256_transform(SHA256_CTX *ctx, const uint8_t *data)
{
    uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

    // initialization 
    for (i = 0, j = 0; i < 16; ++i, j += 4)
        m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
    for ( ; i < 64; ++i)
        m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    for (i = 0; i < 64; ++i)
	{
        t1 = h + EP1(e) + CH(e,f,g) + k[i] + m[i];
        t2 = EP0(a) + MAJ(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

//整体的操作是依次对块进行Hash映射
 void sha256_update(SHA256_CTX *ctx, char *data, unsigned int len)
{
    uint32_t  i;
    for (i = 0; i < len; ++i) 
	{
        ctx->data[ctx->datalen] = data[i]; // 取数据存放到data中，取满64个字节（512 bit）
        ctx->datalen++;
        if (ctx->datalen == 64) 
		{
            // 64 byte = 512 bit  means the buffer ctx->data has fully stored one chunk of message
            // so do the sha256 hash map for the current chunk
            sha256_transform(ctx, ctx->data);
            ctx->bitlen += 512;
            ctx->datalen = 0;
        }
    }
}

/*******************************************************************
********************************************************************
* 名称：void sha256_data_stream_update(SHA256_CTX *ctx, char *data, unsigned int len)
* 功能： 根据要校验的总数据大小，按照64字节进行校验，如果数据没有64字节时，会
搜集数据，然后按照64字节处理，对最后不足64字节数据的，进行尾数处理
* 参数： ctx ：校验辅助结构体   data：要校验的数据   len ：本次要校验的数据长度
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
void sha256_data_stream_update(SHA256_CTX *ctx, char *data, unsigned int len)
{
	u32 multiple_64 = 0 ;  // 保存一次要处理的数据中，64字节一组 ，有多少组
	u8  remainder = 0;     // 保存剩余不足64字节的数据
	if(update_onetime == true)
	{
		ring_buffer_init((stRingBuff *)&sha256_data_storage);
		remain_num = total_len ;   // 刚开始将要处理的数据总数赋值给剩余处理数据
		update_onetime = false;
	}
	write_ring_bufer((stRingBuff *)&sha256_data_storage,data, len);// 将要处理的全部数据写到ringbuf中去
	if(sha256_data_storage.length >= 64)             // 数据要满足>= 64字节
	{
		multiple_64 = sha256_data_storage.length / 64 ; // 对要校验的数据按照64字节取整
		for(uint64_t m = 0 ;m < multiple_64; m++)            // 按照64字节需循环处理一次，这批数据总共要循环处理的次数
		{
			for(int i =0 ;i< 64;i++) // 64字节处理一次
			{
				// 从环形队列中读取一个字节数据，循环64次，保存在辅助处理的结构体中就有64个字节了
				read_one_byte_from_ringbuffer((stRingBuff *)&sha256_data_storage,(char*)& ctx->data[i]); 
				ctx->datalen++ ;
				if (ctx->datalen == 64) // 当满足64个字节时，就可以进行一次校验处理
				{
					sha256_transform(ctx, ctx->data);
					ctx->bitlen += 512;
					ctx->datalen = 0;
				}
			}
			remain_num -=  64; // 将要处理数据的总数-64，因为循环出来后，就表示64个字节已经处理ok
		}

	}
	 // 当要处理的剩余数据少于64字节，且总的要处理的数据又大于等于 64 且环形队列中的有效数据长度和剩余数据相等时，表示是最后的尾数了，进行特殊处理
	if((remain_num < 64) && (total_len >= 64) && (sha256_data_storage.length == remain_num)) 
	{
		remainder = sha256_data_storage.length;  // 将环形队列中的有效数据个数（小于64）取值保存，用于循环读数
		for(unsigned int f = 0 ;f < remainder; f++)
		{
			// 将剩余的不足64字节的数据读取到辅助校验的结构体中
			read_one_byte_from_ringbuffer((stRingBuff *)&sha256_data_storage,(char*)& ctx->data[f]); 
			ctx->datalen++ ;
		}
		remain_num -= remainder ;  //更新剩余处理的数据数量
	}
	// 当要处理的数据总数小于 64 ，且环形队列中已经收到到要处理数据的全部时，开始处理
	if((total_len < 64 )&& (sha256_data_storage.length == total_len)) 
	{
		remainder = sha256_data_storage.length;  // 将环形队列中的有效数据个数（小于64）取值保存，用于循环读数
		for(unsigned int f = 0 ;f < remainder; f++)
		{
			// 将剩余的不足64字节的数据读取到辅助校验的结构体中
			read_one_byte_from_ringbuffer((stRingBuff *)&sha256_data_storage,(char*)& ctx->data[f]); 
			ctx->datalen++ ;
		}
		remain_num -= remainder ; //更新剩余处理的数据数量
	
	}
}



//最后单独甩出的不完整的块需要被补全
void sha256_final(SHA256_CTX *ctx,  char * result_hash)
{
    uint32_t i;            // 用于循环
	char hash[32] = {0} ;   // 用于保存 32字节的hash 值
	char temp[2] = {0} ;   // 暂存 按照%x 打印出来的 2个字符的sha256值
    i = ctx->datalen;

    // Pad whatever data is left in the buffer. 填充缓冲区中剩下的任何数据
    if (ctx->datalen < 56)
	{
        ctx->data[i++] = 0x80;  // pad 10000000 = 0x80
        while (i < 56)
            ctx->data[i++] = 0x00;
    }
    else
	{
        ctx->data[i++] = 0x80;
        while (i < 64)
            ctx->data[i++] = 0x00;
        sha256_transform(ctx, ctx->data);
        memset(ctx->data, 0, 56);
    }

    // Append to the padding the total message's length in bits and transform.
    ctx->bitlen += ctx->datalen * 8;
    ctx->data[63] = ctx->bitlen;
    ctx->data[62] = ctx->bitlen >> 8;
    ctx->data[61] = ctx->bitlen >> 16;
    ctx->data[60] = ctx->bitlen >> 24;
    ctx->data[59] = ctx->bitlen >> 32;
    ctx->data[58] = ctx->bitlen >> 40;
    ctx->data[57] = ctx->bitlen >> 48;
    ctx->data[56] = ctx->bitlen >> 56;
    sha256_transform(ctx, ctx->data); 
        
    // copying the final state to the output hash(use big endian).
    for (i = 0; i < 4; ++i) 
	{
        hash[i]      = (ctx->state[0] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 4]  = (ctx->state[1] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 8]  = (ctx->state[2] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 12] = (ctx->state[3] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 16] = (ctx->state[4] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 20] = (ctx->state[5] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 24] = (ctx->state[6] >> (24 - i * 8)) & 0x000000ff;
        hash[i + 28] = (ctx->state[7] >> (24 - i * 8)) & 0x000000ff;
    }
    for(int i = 0 ; i < 32 ; i++)
	{
	    sprintf(temp,"%02X",hash[i]);	
		strcat(result_hash,temp);
	}
	
//	usart1_printf("SHA256 : %s\r\n",result_hash);
}

/*******************************************************************
********************************************************************
* 名称： void sha256_get(uint8_t *data,uint8_t len,uint8_t *hash)
* 功能： 计算任意字符串的sha256值
* 参数： data： 要计算的字符串指针   
         len： 要计算的字符串的长度  
         result_hash: 保存计算出的最终的hash值的指针 大小 64字节
* 返回值： 无
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
*  result_hash   256 bit   =  为一个32字节大小 ， 一般用64个字节的字符串来表示
SHA256算法中的最小运算单元称为“字”（Word），一个字是32位
*********************************************************************
*********************************************************************/
void sha256_get(char *data,u32 len,char *result_hash)
{
    SHA256_CTX  ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, result_hash);
}

/*******************************************************************
********************************************************************
* 名称：void sha256_get_for_data_stream(char *data,u32 onetime_data_size ,char *sha_256_result)
* 功能：
* 参数：data              : 要校验的数据指针
       onetime_data_size ： 每次要校验的数据包大小（取任意值）
       sha_256_result    ： 保存最终校验的值 64字节字符串

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
void sha256_get_for_data_stream(char *data,u32 onetime_data_size ,char *sha_256_result)
{
	static SHA256_CTX  ctx;                // 一定要使用静态变量（局部）
	static bool onetime2 = true ;          // 执行一次的局部变量
	if(onetime2 == true)                   // 保证 sha256_init(&ctx) 一批数据时，只执行一次
	{
		onetime2 = false;           
		sha256_init(&ctx);                 // 校验数据初始化
		SHA256_CTX_init(ctx);              // 初始化结构体变量，清零
		ring_buffer_init((stRingBuff *)&sha256_data_storage);// 将全局变量清零 (环形队列，保存接收到的数据)
	}
    sha256_data_stream_update(&ctx, data, onetime_data_size); // 对满足64字节的数据 校验
	// 当环形队列中的的有效数据长度为0 且 剩余要处理的数据为0 时
	if(sha256_data_storage.length == 0 && (remain_num == 0) )    // sha256_data_storage.length
	{
		sha256_final(&ctx, sha_256_result); // 最后单独甩出的不完整的块需要被补全
		total_len = 0;                      // 将全局变量清零
		remain_num = 0 ;                        // 将全局变量清零
		onetime2 = true ;                   // 将静态变量清零  
		update_onetime = true;              // 将全局变量清零 （初始化只执行一次的变量清零）
		SHA256_CTX_init(ctx);
		ring_buffer_init((stRingBuff *)&sha256_data_storage);// 将全局变量清零 (环形队列，保存接收到的数据)
	}
}




//void  sha256_printf(unsigned long long max_len,char *data,u32 onetime_data_size )
//{
//	char buf[64] = {0} ;
//	total_len = max_len ; 
//	sha256_get_for_data_stream(data,onetime_data_size ,,buf);
//	usart1_printf("SHA256 : %s\r\n",buf);
//	
//}


