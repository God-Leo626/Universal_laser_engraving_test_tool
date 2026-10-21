#ifndef __MAX49918_H__
#define __MAX49918_H__
#include "stm32f4xx.h"
/************************************************************ */
#define U28_MAX49918_READ_ADDR    0XC9  // U8位号读地址
#define U28_MAX49918_WRITE_ADDR   0XC8  // U8位号写地址
 
#define U29_MAX49918_READ_ADDR    0XCB  // U9位号读地址
#define U29_MAX49918_WRITE_ADDR   0XCA  // U9位号写地址

#define U30_MAX49918_READ_ADDR    0XC9  // U30位号读地址
#define U30_MAX49918_WRITE_ADDR   0XC8  // U30位号写地址

#define U31_MAX49918_READ_ADDR    0XCB  // U31位号读地址
#define U31_MAX49918_WRITE_ADDR   0XCA  // U31位号写地址
/*====================================================================*/
#define GAIN_10      0X00
#define GAIN_20      0X01
#define GAIN_40      0X02
#define bbGAIN_50      0X03
#define GAIN_80      0X04
#define GAIN_100     0X05
#define GAIN_160     0X06
#define GAIN_200     0X07
/*====================================================================*/


void  max49918_iic1_init(void);

void IIC_WriteByte(I2C_TypeDef* i2cx,uint16_t slave_addr,uint8_t write_addr,uint8_t pbuffer);
uint8_t IIC_ReadByte(I2C_TypeDef* i2cx,uint16_t slave_addr,uint8_t read_addr);
void max49918_set_gain(I2C_TypeDef* i2cx,uint16_t ic_select_addr,u8 gain_value);
/*====================================================================*/



#endif

 
