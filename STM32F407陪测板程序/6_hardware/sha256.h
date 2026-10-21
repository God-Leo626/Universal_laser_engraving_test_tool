#ifndef  __SHA256_H__
#define  __SHA256_H__
#include "stm32f4xx.h"
#include <stdint.h>
#include "stdio.h"

//**************************************************************************************

extern volatile uint64_t  total_len ;       // 保存此批要进行sha256校验的数据的总大小
extern volatile uint64_t  remain_num ;          // 保存剩下要处理的数据大小
extern volatile unsigned char hash_buf[65];  // 保存sha256 校验的值 64字符
//**************************************************************************************
typedef   unsigned char BYTE;             // 8位
typedef unsigned int  WORD;             // 32位数据，4字节表示一个字

typedef struct 
{
	BYTE data[64];               // current 512-bit chunk of message data, just like a buffer
	WORD datalen;                // sign the data length of current chunk 对当前块的数据长度进行签名
	unsigned long long bitlen;   // the bit length of the total message
	WORD state[8];               // store the middle state of hash abstract 存储哈希抽象的中间状态
	
} SHA256_CTX;

//**************************************************************************************
#define ROTLEFT(a,b)            (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b)           (((a) >> (b)) | ((a) << (32-(b))))
#define CH(x,y,z)               (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z)              (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)                  (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x)                  (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x)                 (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x)                 (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))


//**************************************************************************************
void SHA256_CTX_init(SHA256_CTX  p_ctx) ;
void sha256_get(char *data,u32 len,char *result_hash);
void sha256_update(SHA256_CTX *ctx, char *data, unsigned int len);
void sha256_get_for_data_stream(char *data,u32 onetime_data_size ,char *sha_256_result);
void  sha256_printf(unsigned long long max_len,char *data,u32 onetime_data_size );

#endif


