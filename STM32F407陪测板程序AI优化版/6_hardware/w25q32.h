#ifndef __W25Q32_H__
#define __W25Q32_H__
#include "stm32f4xx.h"
#include <stdbool.h>
#include "io_bit.h"
#include "spi1.h"
//===========================================================================
extern uint8_t SPI1_FLASH_BUF[4096];
#define W25Q32_CS_PA4   PAout(4)     // 片选
#define W25Q32_CS_PC1   PCout(1)     // 片选
#define DUMMY_DATA  0Xff        //发送的无用数据     

#define W25Q80 	   0XEF13 	
#define W25Q16 	   0XEF14
#define W25Q32 	   0XEF15
#define W25Q64 	   0XEF16



//===========================================================================
//指令表
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg1		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F
//===========================================================================
/* 任务/启动阶段单调用者；每个读写后立即检查结果。禁止ISR调用。
 * 带擦除写以SPI1_FLASH_BUF作内部暂存，源数据必须使用另一个缓冲区。
 * 原void接口保留；参数错误、SPI故障、等待BUSY超时均返回false状态。 */
bool w25q32_last_operation_ok(void);
u8 w25q32_flash_readSR(u8 select) ;
void w25q32_spi_write_disable(u8 select);
u8 w25q32_flash_read_device_id(u8 select);
u32 w25q32_flash_read_jedec_id(u8 select);
void  wait_for_read_ready(u8 select);
void w25q32_flash_read(u8* p_buf,u32 read_addr,u16 num_of_read,u8 select);
void w25q32_flash_page256_write_no_erase(u8* p_buf,u32 write_addr,u16 num_to_write,u8 select);
void w25q32_flash_autopage_write_no_erase(u8* p_buf,u32 write_addr,u32 num_to_write,u8 select);


void w25q32_flash_autosector_write_with_erase(u8* p_buf,u32 write_addr,u32 num_to_write,u8 select);
void w25q32_flash_erase_chip(u8 select);
void w25q32_flash_sector4k_erase(u32 sector_addr,u8 select);
void w25q32_flash_waitfor_write_end(u8 select);
void w25q32_flash_powerdown(u8 select)  ;
void w25q32_flash_wakeup(u8 select)  ;




//===========================================================================
#endif



