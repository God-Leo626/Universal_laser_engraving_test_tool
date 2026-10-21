#ifndef __SPI1_H__
#define __SPI1_H__
#include "stm32f4xx.h"
#include <stdbool.h>
//===========================================================================
extern uint8_t SPI1_FLASH_BUF[4096];
//===========================================================================
void SPI1_SetSpeed(u8 SpeedSet);
void spi1_init(void);
u8 spi1_read_write_byte(u8 data);
//===========================================================================
/* 立即查询最近一个字节的传输结果。 */
bool spi1_last_transfer_ok(void);
#endif


