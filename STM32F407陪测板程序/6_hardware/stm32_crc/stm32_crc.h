#ifndef __STM32_CRC_H__
#define __STM32_CRC_H__
#include "stm32f10x.h"
#include "stdbool.h"
//===========================================================================
#define	CRC32_POLYNOMIAL		((u32)0xEDB88320) //0xedb88320是多项式0x04c11db7按位反转后的值
//#define	RCC_CRC_BIT			((u32)0x00001000)

//===========================================================================
u32 crc32_for_Bytes(u8 *pData,u32 uLen ,bool onetime_cal);

#endif


