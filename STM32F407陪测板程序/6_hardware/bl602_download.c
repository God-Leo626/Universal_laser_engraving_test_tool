#include "bl602_download.h"
#include "module_test.h"
#include "systick_delay_freertos.h"
#include "power_ctrl.h"
#include "usart_dma_all.h"
#include "variable_global.h"
#include "string.h"
#include "lcd.h"
#include "eflash_boot_loader_40m_file.h"
#include "dma.h"
#include "w25q32.h"
#include "my_task.h"
#include "sha256.h"
//**************************************************************************************
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 握手
*********************************************************************
*********************************************************************/
bool bl602_in_burn_mode(void)
{
	uint8_t  data = 0x55;  // 要发送的数据
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	power_ctrl_ext(PWR_3V3_SELECT,false);  //关闭模块的供电电源 3.3v（程序3.3 和 5v 都关闭）
	power_ctrl_ext(PWR_5V_SELECT,false);   // 关闭模块的供电电源  5v （程序3.3 和 5v 都关闭）
	MODULE_BOOT = 1 ;                     // 模块boot脚拉高后上电（ic进入烧录模式）
//	uart4_init(LOAD_BOOT_HEADER_BURN_BAUDRATE);    //初始化波特率的目的是，烧录BootLoader 的前176个字节（后面波特率更新为2000000） 
	systick_delay_ms(500);                 // 等待boot拉高一段时间
	
	power_ctrl_ext(PWR_3V3_SELECT,true);  // 打开模块的供电电源 3.3v（程序3.3 和 5v 都打开，治具上根据模块供电的需要选择合适电压）
	power_ctrl_ext(PWR_5V_SELECT,true);   // 打开模块的供电电源  5v （程序3.3 和 5v 都打开，治具上根据模块供电的需要选择合适电压）
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	systick_delay_ms(500);   // 等待电源稳定
	// 在LOAD_BOOT_HEADER_BURN_BAUDRATE波特率的情况下，要发送100ms数据，要循环的次数7*LOAD_BOOT_HEADER_BURN_BAUDRATE/10000
	for(u16 n = 0; n < 120 * LOAD_BOOT_HEADER_BURN_BAUDRATE / 10000; n++)  
	{
		uart4_dma_tx_send(&data,1); // 给模块发送信息 0x55
	}
	systick_delay_ms(20);
	
	for(u8  m = 0 ;m < 50; m++)              //50次机会
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("mode");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("\r\n");
			if(strstr((const char*)uart4_copy_ring_buffer,"OK") != NULL)   // 表示收到OK回复
			{
				systick_delay_ms(40);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true ;
			}
		}
		systick_delay_ms(5);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("enter boot mode fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"enter boot mode fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
}
/*******************************************************************
********************************************************************
* 名称：void get_boot_info(char* chip_id)
* 功能：获取602的 chipid 8B
* 参数：chip_id：获取到的chipid
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 10 00 00 00
*********************************************************************
*********************************************************************/
bool get_boot_info(char* chipid)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {GET_BOOT_INFO,0x00,0x00,0x00};
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));

	uart4_dma_tx_send(cmd_buf,4);	
	systick_delay_ms(5);
	
	for( u8 m = 0 ; m < 50; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memcpy(bl_602_chip_id,uart4_copy_ring_buffer+16,8);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(5);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("get_boot_info fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"get_boot_info fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
}

/*******************************************************************
********************************************************************
* 名称：bool get_bl602_chipid(char* chip_id)
* 功能： 获取bl602的 chip id  
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool get_bl602_chipid(char* chip_id)
{
	char chip_id_temp_buf[2] = {0};  // 用于将chip id 打印成hex格式时保存中间值
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {GET_BOOT_INFO,0x00,0x00,0x00};
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	
	uart4_dma_tx_send(cmd_buf,4);	
	systick_delay_ms(5);
	
	for( u8 m = 0 ; m < 50; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(bl_602_hex_global_chip_id,0,sizeof(bl_602_hex_global_chip_id));
				memcpy(chip_id,uart4_copy_ring_buffer+16,8);
				for(u8 i=0 ;i<8;i++)
				{	
					sprintf(chip_id_temp_buf,"%02X",bl_602_chip_id[i]);
					strncat(bl_602_hex_global_chip_id,chip_id_temp_buf,2);
				}
				usart1_printf("%s\r\n",bl_602_hex_global_chip_id);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				
				return true;
			}
		}
		systick_delay_ms(5);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("get_bl602_chipid fail\r\n"); 
	lcd_fill_color(10,388,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,388,6,(u8*)"get_bl602_chipid fail",Red,Tan); 

	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 11 00 b0 00
*********************************************************************
*********************************************************************/
bool boot_load_header_176_bytes(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
//	unsigned char cmd_buf[5] = {LOAD_BOOT_HEADER,0x00,0xb0,0x00};
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
//	uart4_dma_tx_send(cmd_buf,4);	
	uart4_dma_tx_send((uint8_t *)bl_bootload_header_table,180);
	
	systick_delay_ms(5);
	
	for( u8 m =0 ;m < 30; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("176");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("\r\n");
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}		
		}
		systick_delay_ms(5);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("boot_load_header_176_bytes fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"boot_load_header_176_bytes fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 17 00 10 00  + 16字节
*********************************************************************
*********************************************************************/
bool load_segment_header(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
//	unsigned char cmd_buf[5] = {LOAD_SEGMENT_HEADER,0x00,0x10,0x00};
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	
//	uart4_dma_tx_send(cmd_buf,4);
	uart4_dma_tx_send((uint8_t *)bl_segment_header_table,20);
	
	systick_delay_ms(50); // 这个时间很重要，要大于50ms
	
	
	for( u8 m =0 ;m < 30; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("segment header");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("\r\n");
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(50);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("load_segment_header fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"load_segment_header fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 18 00 长度N低8位， 长度N高8位， 数据长度N
*********************************************************************
*********************************************************************/
bool load_segment_data(void)
{
	u8 m = 0 ;
	u8 i =0 ;
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
	for( i = 0 ;i < 8;i++)
	{
		if(i < 7) // 当i == 7 的时候，为最后1636个数据的时候
		{
			uart4_dma_tx_send((uint8_t *)bl_segment_data_table[i],4084); // 前面7包数据是 4084
		}
		else //最后一批数据只有 1636（加上命令）
		{
			uart4_dma_tx_send((uint8_t *)bl_segment_data_table[i],1636); // 最后一包数据只有1636
		}
		systick_delay_ms(40);
		for( m =0 ;m < 30; m++ )
		{
			if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
			{
				UART4_REVEIVE_OK = false ;
				read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
				read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//				usart1_printf("%d:data",i);
//				usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//				usart1_printf("\r\n");
				if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
				{
					memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
					break;
				}
			}
			systick_delay_ms(40);
		}
		if( m > 30) // 当 m > 30 的时候，表示从for 循环里出来了都没有收到ok
		{
			// 能运行到这里来，就表示一定是没有接收到想要的信息
			usart1_printf("load_segment_data fail\r\n"); 
			lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
			display_ascii_string(10,360,6,(u8*)"load_segment_data fail",Red,Tan); 
			//==========如果运行到这里，表示芯片烧录失败了==========
			lcd_fill_color(153,220,237,237,Red);
			sprintf((char *)string_buf,"NG");
			display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
			//==========如果运行到这里，表示芯片烧录失败了==========
			memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
			return false ;
		}
	}	
	return true ;

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 19 00 00 00
*********************************************************************
*********************************************************************/
bool check_bootload_image(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {CHECK_IMAGE,0x00,0x00,0x00};
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	
	uart4_dma_tx_send(cmd_buf,4);	// 发送 命令
	
	systick_delay_ms(5);
	
	for( u8 m =0 ;m < 30; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("check");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("\r\n");
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}		
		}
		systick_delay_ms(5);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("check_bootload_image fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"check_bootload_image fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 在 Check image 命令返回 OK 的情况下，通过该命令可以运行下载到 RAM 中的镜像，
  BL602 执行该命令后就从 UART/SDIO
  启动程序跳转到下载的镜像程序中去运行。
* 1a  00  00  00 
*********************************************************************
*********************************************************************/
bool run_bootload_image(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {RUN_IMAGE,0x00,0x00,0x00};
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	// 发送 run imager命令
	uart4_dma_tx_send(cmd_buf,4);	// 发送 命令
	
	systick_delay_ms(5);
	
	for( u8 m =0 ;m < 30; m++ ) // 30 次机会
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("run");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("\r\n");
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(5);	
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("run_bootload_image fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"run_bootload_image fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool change_baud_rate(u32 baudrate)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	uint8_t  data = 0x55;
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
	uart4_close();
	systick_delay_ms(50);//100
	uart4_init(baudrate);
	systick_delay_ms(50);

	// 在LOAD_BOOT_HEADER_BURN_BAUDRATE波特率的情况下，要发送7ms数据，要循环的次数7*LOAD_BOOT_HEADER_BURN_BAUDRATE/10000
	for(u16 n = 0; n < 10 * baudrate / 10000; n++)  
	{
		uart4_dma_tx_send(&data,1); // 给模块发送信息 0x55
	}
	systick_delay_ms(5);
	
	for(u8 m = 0;m < 30;m++)
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("change");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("\r\n");
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				systick_delay_ms(50);  //主机收到“OK”以后，建议延时 20ms 再通信，以防止后续的通信数据与之前的握手数据混在一起
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true ;
			}
		}
		systick_delay_ms(5);
	}
	usart1_printf("change baudrate fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"change baudrate fail",Red,Tan); 
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：该命令用于擦除整片 Flash
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 3c 00 00 00
*********************************************************************
*********************************************************************/
bool  chip_erase(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {CHIP_ERASE,0x00,0x00,0x00};
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
	uart4_dma_tx_send(cmd_buf,4);	// 发送 命令
	
	
	systick_delay_ms(50);
	
	for( u16 m =0 ;m < 150; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_dma_tx_send((uint8_t *)uart4_copy_ring_buffer,read_num); // 将收到的信息打印出来
			
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(100);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("chip_erase fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"chip_erase fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：该命令用于擦除指定地址空间的 Flash。
* 参数：start_address :要擦除的开始地址
        end_address：要擦除结束地址
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 30  [00]   08   00  [4]  [4]
*********************************************************************
*********************************************************************/
bool flash_erase(u32 start_address,u32 end_address)
{
	u32 cmd_temp = start_address;
	unsigned char cmd_buf[13] = {0x30,0x00,0x08,0x00};
	// start address
	cmd_buf[4] = ( (cmd_temp>>0) & 0x00ff );
	cmd_buf[5] = ( (cmd_temp>>8) & 0x00ff );
	cmd_buf[6] = ( (cmd_temp>>16)& 0x00ff );
	cmd_buf[7] = ( (cmd_temp>>24)& 0x00ff );
	
	// end_address
	cmd_temp = end_address;
	cmd_buf[8] =  ( (cmd_temp>>0) & 0x00ff );
	cmd_buf[9] =  ( (cmd_temp>>8) & 0x00ff );
	cmd_buf[10] = ( (cmd_temp>>16)& 0x00ff );
	cmd_buf[11] = ( (cmd_temp>>24)& 0x00ff );
	
	uart4_dma_tx_send(cmd_buf,12);	// 发送 命令
	
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	systick_delay_ms(50);
	
	for( u8 m =0 ;m < 150; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,get_ringbuffer_length_for_usart1(&uart4_receive_buffer)); //将环形队列中的数据读取到一个数组中
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(uart4_copy_ring_buffer,0,strlen(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(100);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("flash_erase fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"flash_erase fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,strlen(uart4_copy_ring_buffer));
	return false ;
	

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：写入 Nbytes 压缩数据到 Flash 指定地址空间
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 每个压缩文件烧录的第一包数据中需要将 Flag 设置为 0x80, 以便告诉 Eflash_-
  loader 需要将压缩引擎初始化, 在后续的数据包中,Flag 需要设置为 0x00
* 3f 00 
*********************************************************************
*********************************************************************/
bool  compressed_data_flash_program(u16 one_time_write_number,u32 bin_length)
{
	u32 m = 0;
	u8 i = 0 ;
	u32 read_num = 0 ;
	u32 read_address = 0; //保存从falsh1 中要读取的地址
	unsigned char cmd_buf[9] = {0x3F,0x00};
	u32 temp = 0 ; // 用于保存发送的长度
	u32  times = 0 ;  // 保存该 待烧录的bin文件按照one_time_write_number烧录需要的次数
	times = bin_length / one_time_write_number ; // 需要烧录多少次
	usart1_printf("number of cycles = [ %u ]\r\n",times);
	
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	memset(flash_readbuf,0,sizeof(flash_readbuf)); // 清0， 保存从flash1 中读取出来的数据
	
	for(m = 0 ;m < times ; m++ )
	{
		usart1_printf("[ %u ]:add = %lu\r\n",m,read_address);
		// 读取要发送的数据
		w25q32_flash_read(flash_readbuf, read_address , one_time_write_number,1); // 从flash1 中读取one_time_write_number 个数据保存到 flash_readbuf
		systick_delay_ms(5);
		if( m == 0) // 第一次发送第一包数据
		{
			temp = 0x80000000 ; // 第一次发送的地址
		}
		else
		{
			temp = m * one_time_write_number ; // 第二次发送的地址
		}
		
		cmd_buf[2] = (  one_time_write_number + 4 )     & 0xff ;
		cmd_buf[3] = (( one_time_write_number + 4 )>>8) & 0xff ;
		
		cmd_buf[4] = ( (temp >> 0) & 0xff );
		cmd_buf[5] = ( (temp >> 8) & 0xff );
		cmd_buf[6] = ( (temp >>16) & 0xff );
		cmd_buf[7] = ( (temp >>24) & 0xff );
		
		// 发送命令
		
		uart4_dma_tx_send(cmd_buf,8); // 发送命令的前8个字节
		uart4_dma_tx_send(flash_readbuf,one_time_write_number);	// 发送命令的 数据部分给待烧录 ic
		
		systick_delay_ms(10);
		
		for(  i = 0;  i < 200 ; i++ )  // 循环次数很重要， 在delay 5 的时候，要取200
		{
			if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
			{
				UART4_REVEIVE_OK = false ;
				read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
				read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
				
				if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
				{
					memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
//					memset(flash_readbuf,0,sizeof(flash_readbuf)); // 清0， 保存从flash1 中读取出来的数据
					read_address += one_time_write_number ;  // 更新下次要读取的地址
					break; // 退出for
				}
			}
			systick_delay_ms(10);
		}
		if(i > 200)  // 执行到这里表示for中没有拿到预期值
		{
			// 能运行到这里来，就表示一定是没有接收到想要的信息
			usart1_printf("compressed_data_flash_program fail\r\n"); 
			lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
			display_ascii_string(10,360,6,(u8*)"compressed_data_flash_program fail",Red,Tan); 
			//==========如果运行到这里，表示芯片烧录失败了==========
			lcd_fill_color(153,220,237,237,Red);
			sprintf((char *)string_buf,"NG");
			display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
			//==========如果运行到这里，表示芯片烧录失败了==========
			memset(uart4_copy_ring_buffer,0,strlen(uart4_copy_ring_buffer));
			memset(flash_readbuf,0,sizeof(flash_readbuf)); // 清0， 保存从flash1 中读取出来的数据	
			return false; 
		}	
	}
	
	if(bin_length % one_time_write_number != 0) //表示有尾数需要处理
	{
		// 读取要发送的数据
		usart1_printf("[ %u ]:add = %lu\r\n",m,read_address);
//		memset(flash_readbuf,0,sizeof(flash_readbuf)); // 清0， 保存从flash1 中读取出来的数据(剩余尾数小于4096)
		w25q32_flash_read(flash_readbuf,read_address,bin_length % one_time_write_number ,1); // 从flash1 中读取one_time_write_number 个数据保存到 UART4_TX_BUF
		systick_delay_ms(5);
		cmd_buf[2] = (  (bin_length % one_time_write_number) + 4 )     & 0xff ;  //lsb
		cmd_buf[3] = (( (bin_length % one_time_write_number) + 4 )>>8) & 0xff ;  // msb
		
		cmd_buf[4] = ( ((m * one_time_write_number) >> 0) & 0xff );
		cmd_buf[5] = ( ((m * one_time_write_number) >> 8) & 0xff );
		cmd_buf[6] = ( ((m * one_time_write_number) >>16) & 0xff );
		cmd_buf[7] = ( ((m * one_time_write_number) >>24) & 0xff );
		
		
		uart4_dma_tx_send(cmd_buf,8); // 发送命令的前8个字节
		uart4_dma_tx_send(flash_readbuf,bin_length % one_time_write_number);	// 发送命令的 数据部分给 ic
		systick_delay_ms(10);
		
		for(  i=0;  i < 200 ; i++ ) // 循环次数很重要， 在delay 5 的时候，要取200
		{
			if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
			{
				UART4_REVEIVE_OK = false ;
				read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
				read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
				if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
				{
					memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
					memset(flash_readbuf,0,sizeof(flash_readbuf)); // 清0， 保存从flash1 中读取出来的数据	
					usart1_printf("[end address] = %lu\r\n",read_address + (bin_length % one_time_write_number));
					return true;
				}
			}
			systick_delay_ms(10);
		}

	}
	else // 刚好要下载的是4096的倍数，没有余数要处理，运行到这里就表示烧录ok
	{
		memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
		memset(flash_readbuf,0,sizeof(flash_readbuf)); // 清0， 保存从flash1 中读取出来的数据
		return true ;
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("compressed_data_flash_program fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"compressed_data_flash_program fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,strlen(uart4_copy_ring_buffer));
	memset(flash_readbuf,0,sizeof(flash_readbuf)); // 清0， 保存从flash1 中读取出来的数据	
	return false ;
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool flash_program_check(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {0x3A,0x00,0x00,0x00};
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	
	uart4_dma_tx_send(cmd_buf,4);	
	systick_delay_ms(20);
	
	for( u8 m = 0 ;m < 150; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("check");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("\r\n");
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(10);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("flash_program_check fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"flash_program_check fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool  chip_sha256_read(u32 start_addr,u32 read_len,char* result_hash)
{
	unsigned char cmd_buf[12] = {0x3D,0x00,0x08,0x00};
	char sha256_buf[32] = {0};
	char temp[2] = {0}; 
	u32 cmd_temp = 0 ;
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
	cmd_temp = start_addr ; 
	cmd_buf[4] = ( (cmd_temp >> 0) & 0xff );
	cmd_buf[5] = ( (cmd_temp >> 8) & 0xff );
	cmd_buf[6] = ( (cmd_temp >>16) & 0xff );
	cmd_buf[7] = ( (cmd_temp >>24) & 0xff );
	
	cmd_temp = start_addr + read_len ;
	cmd_buf[8] =  ( (cmd_temp >> 0) & 0xff );
	cmd_buf[9] =  ( (cmd_temp >> 8) & 0xff );
	cmd_buf[10] = ( (cmd_temp >>16) & 0xff );
	cmd_buf[11] = ( (cmd_temp >>24) & 0xff );
	
	// 发送 sha256_read
	uart4_dma_tx_send(cmd_buf,12);	
	systick_delay_ms(50);	
		
	for( u8 m =0 ;m < 50; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,get_ringbuffer_length_for_usart1(&uart4_receive_buffer)); //将环形队列中的数据读取到一个数组中
			
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"OK") != NULL) // 表示收到ok 回复
			{
				//从uart4_copy_ring_buffer的第5个字符开始copy 32字节到sha256_buf
				memcpy(sha256_buf,uart4_copy_ring_buffer+4,32);
				for(u8 j = 0 ;j < 32;j++)
				{
					 sprintf(temp,"%02x",sha256_buf[j]);
					 strcat(result_hash,temp);
				}
				usart1_printf("ic read sha256 = %s\r\n",result_hash);
				return true ;
			}
		}
		systick_delay_ms(50);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("chip_sha256_read fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"chip_sha256_read fail",Red,Tan); 
	//==========如果运行到这里，表示芯片烧录失败了==========
	lcd_fill_color(153,220,237,237,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;

}
/*******************************************************************
********************************************************************
* 名称：bool chip_sha256_compare(char* chip_sha256,char* result_hash )
* 功能： 比较两个sha256值是否一致
* 参数：chip_sha256：从芯片端获取到的sha256值
		result_hash ：从云端获取到的值
* 返回值： true： 一致
          false： 不一致
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool chip_sha256_compare(char* chip_sha256,char* result_hash )
{
	if(strcmp(chip_sha256,result_hash) == 0)
	{
		return true ;
	}
	else
	{
		usart1_printf("chip_sha256_compare fail\r\n"); 
		lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
		display_ascii_string(10,360,6,(u8*)"chip_sha256_compare fail",Red,Tan); 
		//==========如果运行到这里，表示芯片烧录失败了==========
		lcd_fill_color(153,220,237,237,Red);
		sprintf((char *)string_buf,"NG");
		display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
		return false ;
	}
}
/*******************************************************************
********************************************************************
* 名称：bool module_into_factory(void)
* 功能： 在下载程序ok后，重启电源，让模块进入工厂模式
* 参数：无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 若设备未烧录license数据或License数据文件校验异常时，
默认进入工厂模式，进入工厂模式后，主动发送’factory mode\r\n’告知外界
当前已经进入工厂模式，间隔20ms重复发送’factory mode\r\n’，总计发送3次。
* 
*********************************************************************
*********************************************************************/
bool module_enter_factory_mode(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	// 为了和模块通信进入factory 模式，切换波特率
	uart4_close();
	systick_delay_ms(50);
	uart4_init(AT_COMM_BURN_BAUDRATE);
	systick_delay_ms(50);

	// 给模块重新上电
	MODULE_BOOT = 0 ;                      // 模块boot脚拉拉低，模块正常启动
	systick_delay_ms(100);                  // boot 拉低保持一定时间
	
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	power_ctrl_ext(PWR_5V_SELECT,false);   // 断电重启5v
	power_ctrl_ext(PWR_3V3_SELECT,false);  // 断电重启3.3
	systick_delay_ms(2500);                // 断电保持一定时间
	
	power_ctrl_ext(PWR_5V_SELECT,true);    // 上电5v
	power_ctrl_ext(PWR_3V3_SELECT,true);   // 上电3.3
	systick_delay_ms(10);
	for( u8 m = 0 ;m < 220; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("===%s===\r\n",uart4_copy_ring_buffer); 
			// 没有license 时，直接进入factory mode 并发送字符 factory mode\r\n
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"factory mode\r\nfactory mode") != NULL) // 表示收到 回复  
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
			// 前面是没有license的时候，上电的时候直接进入工厂模式，如果已经烧录了license，就要发送命令
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"factory?\r\n") != NULL)
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				uart4_printf("factory!\r\n"); // 发送进入工厂模式的指令
				break;
			}
		}
		systick_delay_ms(5);
	}
	uart4_printf("factory!\r\n"); // 发送进入工厂模式的指令
	for( u8 m = 0 ;m < 200; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"factory mode\r\n") != NULL) //表示已经进入到 factory mode 中了，所以后面在发送命令就是非法指令  
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(5);
	}

	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("factory mode fail，no firmware or normal mode\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"factory mode fail，no firmware or normal mode",Red,Tan); 
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	lcd_fill_color(153,220,237,237,Red);
//	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,223,6,(u8*)"NG",Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;

}

/*******************************************************************
********************************************************************
* 名称：judge_bool module_into_factory(void)
* 功能： 在下载程序ok后，重启电源，判断模块是否进入工厂模式
* 参数：无
* 返回值：   true ：进入工厂模式
			false ：没有进入工厂模式
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 若设备未烧录license数据或License数据文件校验异常时，
默认进入工厂模式，进入工厂模式后，主动发送’factory mode\r\n’告知外界
当前已经进入工厂模式，间隔20ms重复发送’factory mode\r\n’，总计发送3次。
* 
*********************************************************************
*********************************************************************/
bool judge_module_enter_factory_mode(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	// 为了和模块通信进入factory 模式，切换波特率
	uart4_close();
	systick_delay_ms(50);
	uart4_init(AT_COMM_BURN_BAUDRATE);
	systick_delay_ms(80);

	// 给模块重新上电
	MODULE_BOOT = 0 ;                      // 模块boot脚拉拉低，模块正常启动
	systick_delay_ms(50);                  // boot 拉低保持一定时间
	
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	power_ctrl_ext(PWR_5V_SELECT,false);   // 断电重启5v
	power_ctrl_ext(PWR_3V3_SELECT,false);  // 断电重启3.3
	systick_delay_ms(2000);                // 断电保持一定时间
	
	power_ctrl_ext(PWR_5V_SELECT,true);    // 上电5v
	power_ctrl_ext(PWR_3V3_SELECT,true);   // 上电3.3
	systick_delay_ms(5);
	for( u8 m = 0 ;m < 90; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			// 没有license 时，直接进入factory mode 并发送字符 factory mode\r\n
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"factory mode\r\nfactory mode") != NULL) // 表示收到 回复  
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
			// 前面是没有license的时候，上电的时候直接进入工厂模式，如果已经烧录了license，就要发送命令
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"factory?\r\n") != NULL)
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				uart4_printf("factory!\r\n"); // 发送进入工厂模式的指令
				break;
			}
		}
		systick_delay_ms(10);
	}
	for( u8 m = 0 ;m < 80; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"factory mode\r\n") != NULL) //表示已经进入到 factory mode 中了，所以后面在发送命令就是非法指令  
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(10);
	}

	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("factory mode fail，no firmware or normal mode\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"factory mode fail，no firmware or normal mode",Red,Tan); 
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;

}
/*******************************************************************
********************************************************************
* 名称：bool module_get_fw_ver(void)
* 功能：获取模块的固件版本
* 参数：无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* lcd_fill_color(10,346,312,358,Tan); 
*********************************************************************
*********************************************************************/
bool module_get_fw_ver(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+FW_VER?\r\n"); // 发送获取rssi指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 8; m++ )      // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+FW_VER=ERROR") != NULL) // 表示收到错误回复  
			{
				lcd_fill_color(10,346,312,358,Tan);                                // 清除掉sha256 calculating 位置处的显示
				display_ascii_string(10,346,6,(u8*)"AT+FW_VER=ERROR",Black,Red);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			else  // 读到正确的版本信息的时候
			{
				lcd_fill_color(10,346,312,358,Tan);                                // 清除掉sha256 calculating 位置处的显示
				display_ascii_string(10,346,6,(u8*)uart4_copy_ring_buffer,Black,Green); // 显示版本信息
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			
			}
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到任何信息
	lcd_fill_color(10,346,312,358,Tan);                                // 清除掉sha256 calculating 位置处的显示
	display_ascii_string(10,346,6,(u8*)"AT+FW_VER=ERROR，No information",Black,Red);
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;
}
/*******************************************************************
********************************************************************
* 名称：bool  module_enter_water_lamp(void)
* 功能： 在工厂模式下，发送io流水测试指令
* 参数：无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* lcd_fill_color(10,388,310,400,Tan);
*********************************************************************
*********************************************************************/
bool  module_enter_water_lamp(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	uart4_printf("AT+WATER_LAMP=1\r\n"); // 发送进入硬件io 流水测试指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 10; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			// 支持 io 流水指令的时候
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+WATER_LAMP=OK") != NULL) // 表示收到正确 回复  
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
			// 不支持io流水指令的时候，表示直接进入io流水
			else if (strstr((const char*)uart4_copy_ring_buffer,(const char*)"UNDEFINE") != NULL)
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(80);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf(" AT+WATER_LAMP enter fail\r\n"); 
	lcd_fill_color(10,388,310,400,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,388,6,(u8*)"AT+WATER_LAMP enter fail",Red,Tan); 
	//==========如果运行到这里，表示芯片进入工厂模式失败了==========
	lcd_fill_color(153,238,237,255,Red);
	sprintf((char *)string_buf,"NG");
	display_ascii_string(189,241,6,(u8*)string_buf,Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;
	
}

/*******************************************************************
********************************************************************
* 名称：bool  module_exit_water_lamp(void)
* 功能： 退出流水io 测试指令
* 参数：无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool  module_exit_water_lamp(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+WATER_LAMP=0\r\n"); // 发送进入硬件io 流水测试指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 10; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+WATER_LAMP=OK") != NULL) // 表示收到factory mode 回复  
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(80);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf(" AT+WATER_LAMP exit fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"AT+WATER_LAMP exit fail",Red,Tan); 
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========

	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;
	
}
/*******************************************************************
********************************************************************
* 名称：bool module_get_rssi(void)
* 功能： 获取模块的RSSI值
* 参数：无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 获取rssi时，要注意电源打开后，需要8s左右的时间才能获得rssi值
 且在等待获取时，要循环发送命令后等待读取期望值
* 
*********************************************************************
*********************************************************************/
bool module_get_rssi(void)
{
	u32 read_num = 0 ;       // 保存环形队列中有效数据长度
	char rssi_buf [5] = {0}; // 保存提取出来的rssi的字符（包括符号）
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	for( u8 m = 0 ;m < 40; m++ )       // 这个延时很重要，不能太短
	{
		uart4_printf("AT+WIFI_RSSI?\r\n"); // 发送获取rssi指令
		systick_delay_ms(300);
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("%s\r\n",uart4_copy_ring_buffer);
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+WIFI_RSSI=3") != NULL) // 表示收到 回复  
			{
				usart1_printf("%s\r\n",uart4_copy_ring_buffer);
				// 从第2个","开始找，到后面在出现"," 时，在这个区间的字符串,保存在rssi_buf
				if(capture_string_times_between_two_string(uart4_copy_ring_buffer,rssi_buf,",","," ,2) == true) // 表示rssi在阈值范围内
				{
					return true ;
				}
				else
				{
					usart1_printf(" AT+WIFI_RSSI fail,%s\r\n",uart4_copy_ring_buffer); 
					lcd_fill_color(10,388,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
					display_ascii_string(10,388,6,(u8*)uart4_copy_ring_buffer,Red,Tan); // 在testlog提示区内，显示测试fail 的 rssi实测内容
					
					//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
					lcd_fill_color(153,256,237,273,Red);
					display_ascii_string(189,259,6,(u8*)"NG",Black,Red);   // 显示NG字样
					//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
					memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
					return false ;
				}
			}
			// 表示在没有收到rssi想要的值的时候
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+WIFI_RSSI=1") != NULL)
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
			}
			// 表示在没有收到rssi想要的值的时候
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+WIFI_RSSI=2") != NULL)
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
			}
		}
		systick_delay_ms(200);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf(" AT+WIFI_RSSI fail,%s\r\n",uart4_copy_ring_buffer); 
	lcd_fill_color(10,388,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,388,6,(u8*)"AT+WIFI_RSSI fail",Red,Tan); // 在testlog提示区内，显示测试fail 的 rssi实测内容
	
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	lcd_fill_color(153,256,237,273,Red);
	display_ascii_string(189,259,6,(u8*)"NG",Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;

}


/*******************************************************************
********************************************************************
* 名称：bool module_get_mac(void)
* 功能：获取模块的mac地址
* 参数：无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* lcd_fill_color(10,360,312,372,Tan);     
* 
*********************************************************************
*********************************************************************/
bool module_get_mac(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+DEV_MAC?\r\n"); // 发送获取mac指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 8; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+DEV_MAC=ERROR\r\n") != NULL) // 表示收到错误回复  
			{
				lcd_fill_color(10,360,312,372,Tan);                                // 清除掉mac信息位置处的显示		
				display_ascii_string(10,360,6,(u8*)"AT+DEV_MAC=ERROR",Black,Red);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+DEV_MAC=HARD") != NULL) // 读到正确的mac信息的时候
			{
				lcd_fill_color(10,360,312,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
				display_ascii_string(10,360,6,(u8*)uart4_copy_ring_buffer,Black,Green); // 在testlog提示区内，显示测试pass 的mac实测内容
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到任何信息
	
	lcd_fill_color(10,360,312,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"AT+DEV_MAC? no return ",Red,Tan); // 在testlog提示区内，显示测试fail 的mac实测内容
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool module_get_mac_string(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+DEV_MAC?\r\n"); // 发送获取mac指令
	systick_delay_ms(80);  // 这个延时很重要，不能太短
	for( u8 m = 0 ;m < 8; m++ )  
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+DEV_MAC=ERROR\r\n") != NULL) // 表示收到错误回复  
			{
				lcd_fill_color(10,360,312,372,Tan);                                // 清除掉mac信息位置处的显示		
				display_ascii_string(10,360,6,(u8*)"AT+DEV_MAC=ERROR",Black,Red);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+DEV_MAC=HARD") != NULL) // 读到正确的mac信息的时候
			{
				memset(chip_global_mac,0,sizeof(chip_global_mac));
				// 提取 mac地址字符串并保存
				if(capture_string_skip_n(chip_global_mac,uart4_copy_ring_buffer,11) == true) // 跳过AT+DEV_MAC= 后，保存芯片的mac
				{
					memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
					return true;
				}
				else
				{
					break;
				}
			}
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到任何信息
	
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;

}

/*******************************************************************
********************************************************************
* 名称：bool module_get_model_pcba(void)
* 功能： 获取模块的model
* 参数：无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* lcd_fill_color(10,374,312,400,Tan);  
* 
*********************************************************************
*********************************************************************/
bool module_get_model_pcba(void)
{
u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+MODEL_PCBA?\r\n"); // 发送获取mac指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 8; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+MODEL_PCBA=ERROR\r\n") != NULL) // 表示收到错误回复  
			{
				lcd_fill_color(10,374,312,400,Tan);                                // 清除掉model_pcba信息位置处的显示
				display_ascii_string(10,374,6,(u8*)"AT+MODEL_PCBA=ERROR",Black,Red);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			else  // 读到正确的mac信息的时候
			{
				lcd_fill_color(10,374,312,400,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
				display_ascii_string(10,374,6,(u8*)uart4_copy_ring_buffer,Black,Green); // 在testlog提示区内，显示测试pass 的mac实测内容
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				
				return true;
			}
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到任何信息
	
	lcd_fill_color(10,374,312,400,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,374,6,(u8*)uart4_copy_ring_buffer,Red,Tan); // 在testlog提示区内，显示测试fail 的mac实测内容
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;

}
/*******************************************************************
********************************************************************
* 名称：bool module_get_license(void) 
* 功能：从模块中读取license
* 参数： 无
* 返回值：     真： 有license
              假：无license
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool module_get_license(void)  
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+LICENSE_READ?\r\n"); // 发送获取license指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 20; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+LICENSE_READ=ERROR\r\n") != NULL) // 表示收到 回复  
			{
				lcd_fill_color(153,274,237,291,Red);                   // 把license burn的状态条显示为红色
				display_ascii_string(189,277,6,(u8*)"NG",Black,Red);   // 在状态条显示NG字样
				lcd_fill_color(10,402,312,414,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
				display_ascii_string(10,402,6,(u8*)"NO LICENSE",Red,Tan); // 在testlog提示区内，显示测试fail 的实测内容
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			else if(read_num > 80 )  // 只有读到license的时候，才会返回这么长的字符串长度
			{
				usart1_printf("license information ：%s\r\n",uart4_copy_ring_buffer);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到需要的信息
	lcd_fill_color(153,274,237,291,Red);                   // 把license burn的状态条显示为红色
	display_ascii_string(189,277,6,(u8*)"NG",Black,Red);   // 在状态条显示NG字样
	
	lcd_fill_color(10,402,312,414,Tan);  // 清屏上一次显示的相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,402,6,(u8*)"NO LICENSE",Red,Tan); // 在testlog提示区内，显示测试fail 的实测内容
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;
}
/*******************************************************************
********************************************************************
* 名称：bool module_license_existence_or_not(void)  
* 功能： 判断是否有license 存在
* 参数： 无
* 返回值：true ： 存在   false ：不存在
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool module_license_existence_or_not(void)  
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+LICENSE_READ?\r\n"); // 发送获取license指令
	systick_delay_ms(80);
	
	for( u8 m = 0 ;m < 30; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+LICENSE_READ=ERROR\r\n") != NULL) // 表示收到 回复  
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			else if(read_num > 80 )  // 只有读到license的时候，才会返回这么长的字符串长度
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到需要的信息
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;
}

/*******************************************************************
********************************************************************
* 名称：bool module_license_wirte(char* license)
* 功能： 将license信息拼装成at指令后烧录到模块中
* 参数：license ： 要烧录的licese数据
* 返回值： 真： 成功   假： 失败
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* （准备好了device id 全局变量：license_device_id_global_buf）
* AT+LICENSE_WRITE=<license len>\r\n
功能描述：
	用于写入酷宅license数据文件。 其中<license len>表示需烧录的license文件数据长度
（license文件数据帧参考C1描述，请注意长度应包含 ’\r\n’ ）。此指令操作FLASH. 
若设备处于软AP模式且原未烧录数据，烧录数据后其辐射热点名称不会立刻修改，必须重新启动后再修改。
返回参数：
	1、正常返回：>\r\n
	2、异常返回：AT+LICENSE_WRITE=ERROR\r\n错误标识:错误原因\r\n
*********************************************************************
*********************************************************************/
bool module_license_wirte(void) 
{
	unsigned char compare_hash_buf[65] = {0};       // 保存license 正拼装（去空格）的sha256值，用来后面烧录后的对比
	u32 read_num = 0 ;      // 保存环形队列中有效数据长度
	u16 license_len = 0;    // 保存拼装好的license数据部分的长度
	char license_char_buf[4] = {0}; // 保存license拼装后的字符长度
	char total_license_string_buf[600] = {0};  // 用于保存拼接后的总数据
	char license_assembly_sting_buf[190] = {0};     // 保存要烧录的license 拼装数据（拼接后的数据会小于190）
	char license_buf_temp[140] = {0};     // 保存要烧录的license 拼装数据（拼接后的数据会小于140）
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	
	//========拼接license数据格式，计算出要发送的数据的总长度==============================================================================
    strcat(license_buf_temp,"{\"deviceid\":\"");  // 第一步先拼装license 的头 {"deviceid":"
	strcat(license_buf_temp,license_global_buf);  // 将从云端下载后的license 数据，前面加入{"deviceid":"
															// 将从云端截取到的license 数据里的第一个空格，替换为 ","factory_apikey":"
															// 第2个空格，替换为 ","sta_mac":"
															// 第3个空格，替换为 ","sap_mac":"
															// 第4个空格，替换为 ","device_model":"
															// 第5个空格，替换为 ","device_model":"
	replace_string(license_assembly_sting_buf, license_buf_temp," ", "\",\"factory_apikey\":\"", "\",\"sta_mac\":\"","\",\"sap_mac\":\"","\",\"device_model\":\"","\",\"uiid\":");
	strcat(license_assembly_sting_buf,"}");  // 最后一步拼装license 的尾
//	usart1_printf("license_assembly_sting = %s\r\n",license_assembly_sting_buf);
	license_len = strlen(license_assembly_sting_buf) ; //保存拼装好的 license的长度
//	usart1_printf("license_len = %d\r\n",license_len);
	
	//=========将device id 【放在前面】拼装license，并计算sha256=============================================================================
    delete_char(license_global_buf, ' ');                     // 去掉license 里的空格保存在license_global_buf
	memset(license_device_id_global_buf,0,sizeof(license_device_id_global_buf));
	memcpy(license_device_id_global_buf,license_global_buf,10); // 把去掉空格的云端数据的前10个字符保存到license_device_id_global_buf  （device id）
	
	total_len = strlen(license_global_buf) ;                  // sha256要处理的字符总大小
	memset((void*)hash_buf,0,sizeof(hash_buf));               // 把保存sha256值的全局变量清零
	sha256_get_for_data_stream((char*)license_global_buf,total_len,(char*)hash_buf);
//	usart1_printf("ok licese == %s\r\n",license_global_buf);
	memcpy(compare_hash_buf,(const void*)hash_buf,65);  // 将正常拼装后license（去空格）计算出的sha256保存下来compare_hash_buf
//	usart1_printf("ok sha256 == %s\r\n",compare_hash_buf); // 打印 正拼后的sha256
	
	//=========将device id 【放在最后】拼装license，并计算sha256=============================================================================
    memset((void*)hash_buf,0,sizeof(hash_buf));                 // 把保存sha256值的全局变量清零，后面要使用
	exchange_len_string((char*)hash_buf,license_global_buf,10); // 将license_global_buf里的前10字符移动到后面保存你在hashbuf
//	usart1_printf("exchange license1: %s\r\n",hash_buf);
	memset(license_global_buf,0,sizeof(license_global_buf));
	memcpy(license_global_buf,(const char*)hash_buf,strlen((const char*)hash_buf));
//	usart1_printf("exchange license2:%s\r\n",license_global_buf); // 打印 正拼后的license
	
	total_len = strlen(license_global_buf) ;                    // sha256要处理的字符总大小
	memset((void*)hash_buf,0,sizeof(hash_buf));                 // 把保存sha256值的全局变量清零
	sha256_get_for_data_stream((char*)license_global_buf,total_len,(char*)hash_buf);
	
//	usart1_printf("exchange sha256 : %s\r\n",hash_buf);
	//=========拼装全部需要发送的license=============================================================================
	strcat(total_license_string_buf,"{\"license_frame_len\":");
	sprintf(license_char_buf,"%u",license_len);
	strcat(total_license_string_buf,license_char_buf);// 将拼装好的license 的长度连接到总字符串中
	strcat(total_license_string_buf,",\"license_frame\":");
	strcat(total_license_string_buf,license_assembly_sting_buf);
	strcat(total_license_string_buf,",\"sha256_check\":\"");
	strcat(total_license_string_buf,(const char*)hash_buf);      // 连接sha256
	strcat(total_license_string_buf,"\"}\r\n");                  // 连接末尾
	
	
//	usart1_printf("=>>%s",total_license_string_buf);
//	usart1_printf("AT+LICENSE_WRITE=%u\r\n",strlen(total_license_string_buf)); // 发送license 烧录 指令
	
	
	uart4_printf("AT+LICENSE_WRITE=%u\r\n",strlen(total_license_string_buf)); // 发送license 烧录 指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 8; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)">\r\n") != NULL)  // 读到正确的license 信息的时候 
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));  // 清零全局环形缓冲buf
				break; // 退出此for循环
			}
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+LICENSE_WRITE=ERROR\r\nFAILED OPERATE: LICENSE ALREADY\r\n") != NULL) // 表示收到错误回复  
			{
				lcd_fill_color(10,402,312,414,Tan);                                // 清除掉license 烧写信息位置处的显示
				display_ascii_string(10,402,6,(u8*)"ERROR:LICENSE ALREADY",Black,Red);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				memset((void*)hash_buf,0,sizeof(hash_buf));
				return false;
			}
			// 表示收到错误回复  
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+LICENSE_WRITE=ERROR\r\nFAILED HARDWARE: OTHER HARDWARE\r\n") != NULL)
			{
				lcd_fill_color(10,402,312,414,Tan);                                // 清除掉license 烧写信息位置处的显示
				display_ascii_string(10,402,6,(u8*)"ERROR:OTHER HARDWARE",Black,Red);
				memset((void*)hash_buf,0,sizeof(hash_buf));
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			
		}
		systick_delay_ms(80);
	}
	//===========发送拼接好的license数据=========================================
//	usart1_printf("%s",total_license_string_buf);
	uart4_printf("%s",total_license_string_buf);
	
	systick_delay_ms(100);
	for( u8 m = 0 ;m < 8; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)compare_hash_buf) != NULL) // 从接收到字符串中查找计算好的sha256值，且能找到（表示一致）
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				memset((void*)hash_buf,0,sizeof(hash_buf));
				return true;
			}	
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到任何信息
	
	lcd_fill_color(10,402,312,414,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"FAILED PARAM: FRAMER LEN MISMATCHING") != NULL)
	{
		display_ascii_string(10,402,6,(u8*)"FRAMER LEN MISMATCHING fail",Red,Tan); // 在testlog提示区内，显示固件长度错误
	}
	else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"FRAMER SHA256 CHECK MISMATCHING") != NULL)
	{
		display_ascii_string(10,402,6,(u8*)"FRAMER SHA256 CHECK MISMATCHING fail",Red,Tan); // 在testlog提示区内，显示license sha256错误
	}
	else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"NON COMPLIANCE WITH LICEBSE RULES") != NULL)
	{
		display_ascii_string(10,402,6,(u8*)"NON COMPLIANCE WITH LICEBSE RULES fail",Red,Tan); // 在testlog提示区内，显示不符合规则错误
	}
	else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"DEVICE MODEL MISMATCHING") != NULL)
	{
		display_ascii_string(10,402,6,(u8*)"DEVICE MODEL MISMATCHING fail",Red,Tan); // 在testlog提示区内，device_model与固件不相符
	}
	else
	{
		display_ascii_string(10,402,6,(u8*)"UNKNOWN ERROR fail",Red,Tan); // 在testlog提示区内，未知错误
	}
	usart1_printf("==%s\r\n",uart4_copy_ring_buffer);  // 错误的时候打印出接收到的信息
	memset((void*)hash_buf,0,sizeof(hash_buf));
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;	
}
	
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-02-12
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool module_license_delete(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	uart4_printf("AT+LICENSE_DELETE\r\n"); // 发送获取mac指令
	systick_delay_ms(80);
	for( u8 m = 0 ;m < 8; m++ )  // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中

			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+LICENSE_DELETE=ERROR\r\n") != NULL) // 表示收到错误回复  
			{
				lcd_fill_color(10,388,400,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
				display_ascii_string(10,388,6,(u8*)"AT+LICENSE_DELETE=ERROR",Black,Red);
				
				usart1_printf("[1]LICENSE DELETE fail\r\n");
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return false;
			}
			else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"AT+LICENSE_DELETE=OK\r\n") != NULL) // 读到正确的mac信息的时候
			{
				lcd_fill_color(10,388,400,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
				display_ascii_string(10,388,6,(u8*)"LICENSE DELETE OK",Black,Green);

				usart1_printf("LICENSE DELETE OK\r\n");
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(80);
	}
	// 执行到这里的时候，表示没有收到任何信息
	
	lcd_fill_color(10,388,400,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,388,6,(u8*)"LICENSE DELETE fail",Black,Red);
	usart1_printf("[2]LICENSE DELETE fail\r\n");
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;

}

