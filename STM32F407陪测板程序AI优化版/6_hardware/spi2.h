#ifndef __SPI2_H__
#define __SPI2_H__
#include "stm32f4xx.h"
#include <stdbool.h>
//===========================================================================
extern u8 SPI2_FLASH_BUF[4096];
//===========================================================================
void SPI2_SetSpeed(u8 SpeedSet);
void spi2_init(void);
u8   spi2_read_write_byte(u8 data);

//===========================================================================
/* 立即查询最近一个字节的传输结果。 */
bool spi2_last_transfer_ok(void);
#endif


