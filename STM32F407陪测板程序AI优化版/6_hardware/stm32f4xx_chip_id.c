#include  "stm32f4xx_chip_id.h"
#include "stm32f4xx.h"
#include "usart_dma_all.h"
#include "string.h"
#include "stm32_crc.h"
#include "lcd.h"
//**************************************************************************************

/*******************************************************************
********************************************************************
* 名称：void stm32f10x_get_chip_id ( char * mac_buf )
* 功能： 读取芯片ID  将id 进行crc32校验， 模拟mac并打印
* 参数： mac_buf_12 :提供mac保存的空间 大小12 字节
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
void stm32f4xx_print_chip_id_crc_mac( char * mac_buf_12 )
{
	char id_buf[25] = {0};
	char crc4_buf[10] = {0};
	u32  crc32_temp = 0;
	u32 CpuID[3] = {0};
								//小端模式
    CpuID[0] = * ( vu32 * ) ( 0x1FFF7A10 ); //高32位地址
    CpuID[1] = * ( vu32 * ) ( 0x1FFF7A14 ); //中32位地址
    CpuID[2] = * ( vu32 * ) ( 0x1FFF7A18 ); //低32位地址

    usart1_printf ( "[== CHIP ID 24 ==]: %08X%08X%08X\r\n",CpuID[2],CpuID[1],CpuID[0]); // 串口1打印chip id
	sprintf(id_buf,"%08X%08X%08X",CpuID[2],CpuID[1],CpuID[0]);                       // 将chip  id 打印成一个字符串
	crc32_temp = crc32_for_Bytes((u8 *)id_buf,strlen(id_buf) ,0);                    // crc32 校验值,4字节 8个字符
	usart1_printf("[== CRC32 8 ==]: %X\r\n",crc32_temp);                               //  输出 chip id 的 crc32 校验值
	sprintf(crc4_buf,"%08X",CpuID[0]);                                               // 将CpuID[0]中的值，取8个字符
	memset(id_buf,0,25);    //清零id_buf                                                         // 清零
	strncpy(id_buf,crc4_buf,4);                                                      // 截取4个字符保存
	sprintf(mac_buf_12,"%08X%s",crc32_temp,id_buf);                                     // crc32 校验4字节 8字符 + 8字符= 16字符  
	display_ascii_string(153,72,8,(unsigned char *)mac_buf_12,Blue,Tan);    
	sprintf(mac_buf_12,"%08X%s",crc32_temp,id_buf);                                     // crc32 校验4字节 8字符 + 8字符= 16字符  
	usart1_printf("[== MAC 12 ==]: %s\r\n",mac_buf_12);                                    // 串口1打印MAC 校验值
}
/*******************************************************************
********************************************************************
* 名称：void stm32f10x_get_mac ( char * mac_buf )
* 功能： 根据chip id 生产 mac 并保存到 mac_buf 中
* 参数： mac_buf ： 接收mac
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
void stm32f4xx_get_mac ( char * mac_buf )
{
	char id_buf[25] = {0};
	char crc4_buf[10] = {0};
	u32  crc32_temp = 0;
	u32 CpuID[3];
								//小端模式
    CpuID[0] = * ( vu32 * ) ( 0x1FFF7A10 ); //高32位地址
    CpuID[1] = * ( vu32 * ) ( 0x1FFF7A14 ); //中32位地址
    CpuID[2] = * ( vu32 * ) ( 0x1FFF7A18 ); //低32位地址

	sprintf(id_buf,"%08X%08X%08X",CpuID[2],CpuID[1],CpuID[0]);                       // 将chip  id 打印成一个字符串
	crc32_temp = crc32_for_Bytes((u8 *)id_buf,strlen(id_buf) ,0);                    // crc32 校验值,4字节 8个字符
	sprintf(crc4_buf,"%08X",CpuID[0]);                                               // 将CpuID[0]中的值，取8个字符
	memset(id_buf,0,25);                                                             // 清零
	strncpy(id_buf,crc4_buf,4);                                                      // 截取4个字符保存
	sprintf(mac_buf,"%08X%s",crc32_temp,id_buf);                                     // crc32 校验4字节 8字符 + 8字符= 16字符  
//	usart1_printf("[== MAC ==]: %s\r\n",mac_buf);                                    // 串口1打印MAC 校验值
	
	
}
/*******************************************************************
********************************************************************
* 名称：void stm32f4xx_get_flash_size ( void )
* 功能：获取芯片Flash大小(单位k字节)
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 0x1FFF7A22 为保存内部flash 的大小
*********************************************************************
*********************************************************************/
void stm32f4xx_get_flash_size ( void )
{
    u16 stm32_Flash_Size;
    stm32_Flash_Size = * ( u16 * ) ( 0x1FFF7A22 ); //闪存容量寄存器
    usart1_printf ( "[ ==STM32 FLASH SIZE == ]: %u K\r\n", stm32_Flash_Size );
}
/*******************************************************************
********************************************************************
* 名称：void stm32f10x_get_flash_main_chip_type(void)
* 功能： 读取芯片型号
* 参数：
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
//void stm32f4xx_get_flash_main_chip_type(void)
//{
//	u32 type;
//	type = *( unsigned  int * )( 0xE0042000 );	//MCU ID
//	usart1_printf ( "芯片型号为:0x%x \r\n", type );
//}




 
 
 







