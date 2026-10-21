#include "bl702_download.h"
#include "module_test.h"
#include "systick_delay_freertos.h"
#include "power_ctrl.h"
#include "usart_dma_all.h"
#include "variable_global.h"
#include "string.h"
#include "lcd.h"
#include "eflash_702boot_loader_40m_file.h"
#include "dma.h"
#include "w25q32.h"
#include "my_task.h"
#include "sha256.h"
#include "aes.h"

//*********************************************************************************************************************
/*******************************************************************
********************************************************************
* 名称：  bool bl702_in_burn_mode(void)
* 功能：  发送55数据，和串口同步波特率，进入烧录模式
* 参数：
* 返回值：true  : 进入烧录模式
         false : 进入烧录模式失败
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool  bl702_in_burn_mode(void)
{
	uint8_t  data = 0x55;                  // 要发送的数据
	u32 read_num = 0 ;                     // 保存环形队列中有效数据长度
	power_ctrl_ext(PWR_3V3_SELECT,false);  //  关闭模块的供电电源 3.3v（程序3.3 和 5v 都关闭）
	power_ctrl_ext(PWR_5V_SELECT,false);   // 关闭模块的供电电源  5v （程序3.3 和 5v 都关闭）
	MODULE_BOOT = 1 ;                      // 模块boot脚拉高后上电（ic进入烧录模式）
//	uart4_init(LOAD_BOOT_HEADER_BURN_BAUDRATE);    //初始化波特率的目的是，烧录BootLoader 的前176个字节（后面波特率更新为2000000） 
	systick_delay_ms(500);                 // 等待boot拉高一段时间
	
	power_ctrl_ext(PWR_3V3_SELECT,true);   // 打开模块的供电电源 3.3v（程序3.3 和 5v 都打开，治具上根据模块供电的需要选择合适电压）
	power_ctrl_ext(PWR_5V_SELECT,true);    // 打开模块的供电电源  5v （程序3.3 和 5v 都打开，治具上根据模块供电的需要选择合适电压）
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	systick_delay_ms(500);   // 等待电源稳定
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	// 在BL702_LOAD_BOOT_HEADER_BURN_BAUDRATE波特率的情况下，要发送100ms数据，要循环的次数7*LOAD_BOOT_HEADER_BURN_BAUDRATE/10000
	for(u16 n = 0; n < 120 * BL702_LOAD_BOOT_HEADER_BURN_BAUDRATE / 10000; n++)  
	{
		uart4_dma_tx_send(&data,1); // 给模块发送信息 0x55
	}
	systick_delay_ms(10);
	
	for(u8  m = 0 ;m < 60; m++)              //50次机会
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
	usart1_printf("bl702 enter boot mode fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702 enter boot mode fail",Red,Tan); 
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
* 名称： bool bl702_get_boot_info(char* chipid)
* 功能： 获取bl702的 chip id 并保存在全局变量bl_702_chip_id
* 参数： 无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_get_boot_info(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {BL702_GET_BOOT_INFO,0x00,0x00,0x00};
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
    memset(bl_702_chip_id,0,sizeof(bl_702_chip_id));
    
	uart4_dma_tx_send(cmd_buf,4);	// 发送命令
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
				// 从 接收到的字符串中的第20个字节后开始，copy 8个字节数据，并保持在全局变量bl_702_chip_id
				memcpy(bl_702_chip_id,uart4_copy_ring_buffer + 20,8);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				return true;
			}
		}
		systick_delay_ms(5);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("bl702_get_boot_info fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_get_boot_info fail",Red,Tan); 
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
* 名称：bool get_bl702_chipid(void)
* 功能：获取bl702的chip id 以字符形式保存在全局变量 bl_702_chip_id中
        且以hex的格式%02X保持在全局变量 bl_702_hex_global_chip_id  中
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool get_bl702_chipid(void)
{
	char chip_id_temp_buf[2] = {0};  // 用于将chip id 打印成hex格式时保存中间值
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {BL702_GET_BOOT_INFO,0x00,0x00,0x00};
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
				memset(bl_702_hex_global_chip_id,0,sizeof(bl_702_hex_global_chip_id));
				memcpy(bl_702_chip_id,uart4_copy_ring_buffer + 20,8);
				for(u8 i=0 ;i<8;i++)
				{	
					sprintf(chip_id_temp_buf,"%02X",bl_702_chip_id[i]);
					strncat(bl_702_hex_global_chip_id,chip_id_temp_buf,2);
				}
				usart1_printf("bl702 chip id = %s\r\n",bl_702_hex_global_chip_id);
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				
				return true;
			}
		}
		systick_delay_ms(5);
	}
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("get_bl702_chipid fail\r\n"); 
	lcd_fill_color(10,388,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,388,6,(u8*)"get_bl702_chipid fail",Red,Tan); 

	//==========如果运行到这里，表示芯片烧录失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false ;
	
}
/*******************************************************************
********************************************************************
* 名称：bool bl702_boot_load_header_176_bytes(void)
* 功能：  导入702 的 bootload 的前 180字节  Load boot header
* 参数： 无
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_boot_load_header_176_bytes(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
//	unsigned char cmd_buf[5] = {LOAD_BOOT_HEADER,0x00,0xb0,0x00};
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
//	uart4_dma_tx_send(cmd_buf,4);	
	uart4_dma_tx_send((uint8_t *)bl702_bootload_header_table,180);
	
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
	usart1_printf("bl702_boot_load_header_176_bytes fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_boot_load_header_176_bytes fail",Red,Tan); 
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
* 名称： bool bl702_load_segment_header(void)
* 功能： 继续烧录 Load Segment Header
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_load_segment_header(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
//	unsigned char cmd_buf[5] = {LOAD_SEGMENT_HEADER,0x00,0x10,0x00};
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	
//	uart4_dma_tx_send(cmd_buf,4);
	uart4_dma_tx_send((uint8_t *)bl702_segment_header_table,20);
	
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
	usart1_printf("bl702_load_segment_header fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_load_segment_header fail",Red,Tan); 
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
* 名称：  bool bl702_load_segment_data(void)
* 功能：  将702的剩下的bootload全部烧录到ram中去
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_load_segment_data(void)
{
	u8 m = 0 ;
	u8 i =0 ;
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
	for( i = 0 ;i < 15;i++) // 有15行数据
	{
		if(i < 14) // 当i == 14 的时候，为最后1812个数据的时候
		{
			uart4_dma_tx_send((uint8_t *)bl702_segment_data_table[i],4084); // 前面7包数据是 4084
		}
		else //最后一批数据只有 1812（加上命令）
		{
			uart4_dma_tx_send((uint8_t *)bl702_segment_data_table[i],1812); // 最后一包数据只有1812
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
		if( m >= 30) // 当 m > 30 的时候，表示从for 循环里出来了都没有收到ok
		{
			// 能运行到这里来，就表示一定是没有接收到想要的信息
			usart1_printf("bl702_load_segment_data fail\r\n"); 
			lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
			display_ascii_string(10,360,6,(u8*)"bl702_load_segment_data fail",Red,Tan); 
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
* 名称： bool check_bootload_image(void)
* 功能： 校验702的bootload的下载的完整性
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_check_bootload_image(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {BL702_CHECK_IMAGE,0x00,0x00,0x00};
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
	usart1_printf("bl702_check_bootload_image fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_check_bootload_image fail",Red,Tan); 
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
* 名称：  bool bl702_run_bootload_image(void)
* 功能： 运行下载到ram中的bootload
* 参数：
* 返回值：  true ： 运行ok   false ：运行失败
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_run_bootload_image(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {BL702_RUN_IMAGE,0x00,0x00,0x00};
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
	usart1_printf("bl702_run_bootload_image fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_run_bootload_image fail",Red,Tan); 
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
* 名称：bool bl702_change_baud_rate(u32 baudrate)
* 功能：更新串口4的波特率，为后面高速的下载bootload准备
* 参数：baudrate ：要更新的波特率
* 返回值： true ：更新完成  false ：更新失败
* 版本：v1
* 作者：matt
* 日期：2023-06-03
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool bl702_change_baud_rate(u32 baudrate)
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
	systick_delay_ms(2);
	
	for(u8 m = 0; m < 30;m++)
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer);
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("==========\r\n");
//			usart1_dma_tx_send((uint8_t*)&uart4_copy_ring_buffer,read_num);
//			usart1_printf("==========\r\n");
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
* 名称：  bool  bl702_chip_erase(void)
* 功能：  702全片擦除flash
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool  bl702_chip_erase(void)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[5] = {BL702_CHIP_ERASE,0x00,0x00,0x00};
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	ring_buffer_init_for_usart1(&uart4_receive_buffer); //  	清理环形缓冲器
	
	uart4_dma_tx_send(cmd_buf,4);	// 发送 命令
	
	systick_delay_ms(50);
	
	for( u16 m = 0 ;m < 150; m++ )
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
	usart1_printf("bl702_chip_erase fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_chip_erase fail",Red,Tan); 
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
* 名称： bool  bl702_flash_program(u16 one_time_write_number,u32 bin_length)
* 功能： 将要烧录的固件烧录到ic的flash区域
* 参数： one_time_write_number ：  单次写入的固件数据大小
		bin_length                要写入的固件总大小
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool  bl702_flash_program(u16 one_time_write_number,u32 bin_length)
{
	u32 m = 0;
	u8 i = 0 ;
	u32 read_num = 0 ;
	u32 read_address = 0; //保存从falsh1 中要读取的地址
	unsigned char cmd_buf[9] = {0x31,0x00};
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
		
		temp = m * one_time_write_number ; // 固件要烧录的开始地址
        
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
			usart1_printf("bl702_flash_program fail\r\n"); 
			lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
			display_ascii_string(10,360,6,(u8*)"bl702_flash_program fail",Red,Tan); 
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
	usart1_printf("bl702_flash_program fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_flash_program fail",Red,Tan); 
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
* 名称：bool bl702_flash_program_check(void)
* 功能：确认 Flash 数据烧写过程中是否出现错误
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_flash_program_check(void)
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
	usart1_printf("bl702_flash_program_check fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_flash_program_check fail",Red,Tan); 
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
* 名称：bool  bl702_chip_sha256_read(u32 start_addr,u32 read_len,char* result_hash)
* 功能： 获取ic flash中烧录固件的sha256值,并以%02x的格式保存在变量中
* 参数：start_addr ：sha256校验 时，flash开始读取的位置
		read_len   ：读取的长度
		result_hash ：保存校验的sha256的 十六进制值
* 返回值： true ： 读取sha256 ok
           false ： 读取sha256失败
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool  bl702_chip_sha256_read(u32 start_addr,u32 read_len,char* result_hash)
{
	u32 read_num = 0 ;   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[12] = {0x3D,0x00,0x08,0x00};  // 读取sha256 的命令
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
	
	// 发送 sha256_read 命令
	uart4_dma_tx_send(cmd_buf,12);	
	systick_delay_ms(50);	
		
	for( u8 m =0 ;m < 50; m++ )
	{
		if(UART4_REVEIVE_OK == true) // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			
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
	usart1_printf("bl702_chip_sha256_read fail\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"bl702_chip_sha256_read fail",Red,Tan); 
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
* 名称：bool bl702_chip_sha256_compare(char* chip_sha256,char* result_hash )
* 功能： 比较两个sha256值是否一致
* 参数：chip_sha256：从芯片端获取到的sha256值
		result_hash ：从云端获取到的值
* 返回值： true： 一致
          false： 不一致
* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 
* 
*********************************************************************/
bool bl702_chip_sha256_compare(char* chip_sha256,char* result_hash )
{
	if(strcmp(chip_sha256,result_hash) == 0)
	{
		return true ;
	}
	else
	{
		usart1_printf("bl702_chip_sha256_compare fail\r\n"); 
		lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
		display_ascii_string(10,360,6,(u8*)"bl702_chip_sha256_compare fail",Red,Tan); 
		//==========如果运行到这里，表示芯片烧录失败了==========
		lcd_fill_color(153,220,237,237,Red);
		sprintf((char *)string_buf,"NG");
		display_ascii_string(189,223,6,(u8*)string_buf,Black,Red);   // 显示NG字样
		return false ;
	}
}

/*******************************************************************
********************************************************************
* 名称：bool bl702_judge_module_activate_factorymode(void)
* 功能：bl702模块是否已经激活和进入工厂模式
        
* 参数：无
* 返回值：
true  : 表示进入到工厂模式
flase： 没有进入到工厂模式

bl702_global_activate_flag ： true  表示已经激活
bl702_global_activate_flag ： false 表示没有激活

* 版本：v1
* 作者：matt
* 日期：2023-05-26
* 举例：
* 备注：
* 空片测试时，一定是进入不了工厂模式，返回false
  当返回true时，就要进一步判断 bl702_global_activate_flag 是否为真，为真，表示已经激活
  不为真，表示没有激活（有固件，没有激活）
* 
*********************************************************************/
bool bl702_judge_module_activate_factorymode(void)
{
	bl702_global_activate_flag = false;  // 没有激活，默认没有激活，
	u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[6] = {0x7F,0x06,0x01,0x00,0x80,0x87}; //  设备进入产测模式的命令
//	unsigned char cmd_response_buf[5] = {0x7F,0x07,0x01,0x80,'\0'} ; //  进入工厂模式后的返回值的前4个字节
	unsigned char factory_mode_flag = 0x11;                          //  保存进入工厂模式标志 00 进入工厂，其他失败
	unsigned char_campare_buf[14] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	// 模块上电打印数据，需要2000000的波特率用来接收上电模块丢出的数据
    power_ctrl_ext(PWR_5V_SELECT,false);    // 断电5v
	power_ctrl_ext(PWR_3V3_SELECT,false);   // 断电3.3
    MODULE_BOOT = 0 ;                       // 模块boot脚拉低后上电（ic进入正常模式），这个很重要
    
	uart4_close();
	systick_delay_ms(50);
	uart4_init(BL702_MODULE_POWERON_COMM_BAUDRATE); // 切换为702模块上电时候的串口波特率，用来接收模块上电打印的数据 2000000
	systick_delay_ms(100);

	// 给模块上电
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	power_ctrl_ext(PWR_5V_SELECT,true);    // 上电5v
	power_ctrl_ext(PWR_3V3_SELECT,true);   // 上电3.3
	systick_delay_ms(8);
	for(u16 m = 0 ; m < 300; m++ )        // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)       // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("===%s\r\n",uart4_copy_ring_buffer);
			// 702模块有固件，有激活过，会上电打印$AUTH:1
			if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"$AUTH:1") != NULL) // 收到$AUTH:1，表示模块有固件且已经激活
			{
//				uart4_dma_tx_send(cmd_buf,6);	  // 发送进入工厂模式的 命令
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				bl702_global_activate_flag = true;  // 已经激活
				usart1_printf("bl702 module is activated\r\n");
				break;
			}
            else if(strstr((const char*)uart4_copy_ring_buffer,(const char*)"$AUTH:0") != NULL) // 收到$AUTH:0，表示模块有固件但没有激活
			{
				memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
				usart1_printf("bl702 module is not active\r\n");
				break;
			}
		}
		systick_delay_ms(4);
		if(m == 300)  // 当 m == 300 的时候，表示前面任然没有收到对应的数据，在重启一下电源
		{
			power_ctrl_ext(PWR_5V_SELECT,false);    // 关闭5v
			power_ctrl_ext(PWR_3V3_SELECT,false);   // 关闭3.3
			systick_delay_ms(100);
			power_ctrl_ext(PWR_5V_SELECT,true);     // 上电5v
			power_ctrl_ext(PWR_3V3_SELECT,true);    // 上电3.3
			systick_delay_ms(2);
		}
	}
    for( u16 m = 0 ;m < 70; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 7 ) // 准备进入工厂模式后的返回值有7个字节
			{
                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
				if(strstr((const char*)char_campare_buf,(const char*)"7F-07-01-90") != NULL) //表示已经准备好进入工厂模式， 收到了：0x7F,0x07,0x01,0x90
                {
                    memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer)); // 清空串口4的接收buffer，以备后续继续接收进入工厂模式后的返回值
                    uart4_dma_tx_send(cmd_buf,6);	  // 发送进入工厂模式的 命令
                    usart1_printf("bl702 get ready to enter factory mode\r\n"); 
                    break;
                }
			}
		}
		systick_delay_ms(2);
	}
    uart4_dma_tx_send(cmd_buf,6);	  // 发送进入工厂模式的 命令
    for( u8 m = 0 ;m < 50; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 7 ) // 已经进入工厂模式后的返回值有7 个字节
			{
                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
				if(strstr((const char*)char_campare_buf,(const char*)"7F-07-01-80") != NULL) //表示已经进入工厂模式， 收到了：0x7F,0x07,0x01,0x80
                {
                    // 从串口接收到的7个数据中，从第6个开始copy 1个到  factory_mode_flag
                    memcpy(&factory_mode_flag,uart4_copy_ring_buffer + 5,1);
                    if(factory_mode_flag == 0x00)     //   factory_mode_flag == 0, 表示进入到factory mode
                    {
                        memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                        return true ;
                    }         
                }
			}
		}
		systick_delay_ms(10);
	}
    
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("bl702_factory mode fail，no firmware or no dongle\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"factory mode fail，no firmware or no dongle",Red,Tan); 
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;

}
/*******************************************************************
********************************************************************
* 名称：bool bl702_enter_factorymode(void)
* 功能：判断BL702的模块有没有进入到工厂模式
* 参数：无
* 返回值： true ： 进入到工厂模式
          false ： 没有进入到工厂模式
* 版本：v1
* 作者：matt
* 日期：2023-06-03
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
//bool bl702_enter_factorymode(void)
//{
//	u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
//	unsigned char cmd_buf[6] = {0x7F,0x06,0x01,0x00,0x80,0x87}; //  设备进入产测模式的命令
////	unsigned char cmd_response_buf[5] = {0x7F,0x07,0x01,0x80,'\0'} ; //  进入工厂模式后的返回值的前4个字节
//	unsigned char factory_mode_flag = 0x11;                     //  保存进入工厂模式标志 00 进入工厂，其他失败
//	unsigned char_campare_buf[14] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
//	// 模块上电打印数据，需要2000000的波特率用来接收上电模块丢出的数据
//    power_ctrl_ext(PWR_5V_SELECT,false);    // 断电5v
//	power_ctrl_ext(PWR_3V3_SELECT,false);   // 断电3.3
//    MODULE_BOOT = 0 ;                       // 模块boot脚拉低后上电（ic进入正常模式），这个很重要
//    
//	uart4_close();
//	systick_delay_ms(50);
//	uart4_init(BL702_MODULE_POWERON_COMM_BAUDRATE); // 切换为702模块上电时候的串口波特率，用来接收模块上电打印的数据 2000000
//	systick_delay_ms(200);

//	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
//	// 给模块上电
//	power_ctrl_ext(PWR_5V_SELECT,true);    // 上电5v
//	power_ctrl_ext(PWR_3V3_SELECT,true);   // 上电3.3
//    systick_delay_ms(8);
//    
//    for( u16 m = 0 ;m < 400; m++ )    // 这个延时很重要，不能太短
//	{
//		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
//		{
//			UART4_REVEIVE_OK = false ;
//			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
//			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("===%s\r\n",uart4_copy_ring_buffer);
//            if( read_num >= 7 ) // 准备进入工厂模式后的返回值有7个字节
//			{
//                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
//				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
//				if(strstr((const char*)char_campare_buf,(const char*)"7F-07-01-90") != NULL) //表示已经准备好进入工厂模式， 收到了：0x7F,0x07,0x01,0x90
//                {
//                    memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer)); // 清空串口4的接收buffer，以备后续继续接收进入工厂模式后的返回值
//                    uart4_dma_tx_send(cmd_buf,6);	  // 发送进入工厂模式的 命令
//                    usart1_printf("bl702 get ready to enter factory mode\r\n"); 
//                    break;
//                }
//			}
//		}
//		systick_delay_ms(5);
//	}
//    uart4_dma_tx_send(cmd_buf,6);	  // 发送进入工厂模式的 命令
//    for( u8 m = 0 ;m < 100; m++ )    // 这个延时很重要，不能太短
//	{
//		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
//		{
//			UART4_REVEIVE_OK = false ;
//			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
//			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			if( read_num >= 7 ) // 已经进入工厂模式后的返回值有7 个字节
//			{
//                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
//				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
//				if(strstr((const char*)char_campare_buf,(const char*)"7F-07-01-80") != NULL) //表示已经进入工厂模式， 收到了：0x7F,0x07,0x01,0x80
//                {
//                    // 从串口接收到的7个数据中，从第6个开始copy 1个到  factory_mode_flag
//                    memcpy(&factory_mode_flag,uart4_copy_ring_buffer + 5,1);
//                    if(factory_mode_flag == 0x00)     //   factory_mode_flag == 0, 表示进入到factory mode
//                    {
//                        memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
//                        return true ;
//                    }         
//                }
//			}
//		}
//		systick_delay_ms(5);
//	}
//    
//	// 能运行到这里来，就表示一定是没有接收到想要的信息
//	usart1_printf("bl702_factory mode fail，no firmware or no dongle\r\n"); 
//	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
//	display_ascii_string(10,360,6,(u8*)"factory mode fail，no firmware or no dongle",Red,Tan); 
//	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
//	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
//	return false;

//}

bool bl702_enter_factorymode(void)
{
	u8 factory_buf[8] = {0x7f,0x07,0x01,0x90};  // 保存准备好进入工程模式时的返回值
	u16 k = 0, i= 0 ,m=0;                           // 用于两个for循环
	u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
	u32 read_num_total =  0 ;            // 计数读取的总字节数
	unsigned char cmd_buf[6] = {0x7F,0x06,0x01,0x00,0x80,0x87}; //  设备进入产测模式的命令
//	unsigned char cmd_response_buf[5] = {0x7F,0x07,0x01,0x80,'\0'} ; //  进入工厂模式后的返回值的前4个字节
	unsigned char factory_mode_flag = 0x11;                     //  保存进入工厂模式标志 00 进入工厂，其他失败
	unsigned char_campare_buf[14] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
	// 模块上电打印数据，需要2000000的波特率用来接收上电模块丢出的数据
    power_ctrl_ext(PWR_5V_SELECT,false);    // 断电5v
	power_ctrl_ext(PWR_3V3_SELECT,false);   // 断电3.3
    MODULE_BOOT = 0 ;                       // 模块boot脚拉低后上电（ic进入正常模式），这个很重要
    
	uart4_close();
	systick_delay_ms(50);
	uart4_init(BL702_MODULE_POWERON_COMM_BAUDRATE); // 切换为702模块上电时候的串口波特率，用来接收模块上电打印的数据 2000000
	systick_delay_ms(200);

	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	// 给模块上电
	power_ctrl_ext(PWR_5V_SELECT,true);    // 上电5v
	power_ctrl_ext(PWR_3V3_SELECT,true);   // 上电3.3
    systick_delay_ms(8);
    
    for( m = 0 ;m < 300; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
//			usart1_printf("===%s\r\n",uart4_copy_ring_buffer);
			read_num_total += read_num ;
            if( read_num_total >= 128 ) // 当总的接收字符数量达到150个的时候，开始判断
			{
				for( i = 0 ; i< read_num_total;i++) // 从 0----环形形缓冲接收到的总字符个数 中找值 0x7f
				{
					if(uart4_copy_ring_buffer[i] == 0x7f)
					{
						for( k = 0 ; k < 4 ;k++)
						{
//							usart1_printf("uart4_copy_ring_buffer[%d] = %02x\r\n",i+k,uart4_copy_ring_buffer[i + k]);
							if(uart4_copy_ring_buffer[i + k] != factory_buf[k])
							{
								break;					
							}								
						}
						if(k >= 4)  // for 循环中找到所有匹配的值
						{
							uart4_dma_tx_send(cmd_buf,6);	
							 m = 400 ; // 当收到需要的值以后，将循环变量M = 400,使其条件不成立，直接退出下一次循环	
                            break;							
						}
						else
						{
							usart1_printf("bl702_factory mode fail，no firmware or no dongle\r\n"); 
							lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
							display_ascii_string(10,360,6,(u8*)"factory mode fail，no firmware or no dongle",Red,Tan); 
							//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
							memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
							return false;
						}
					}
				}
			}
		}
//		if(k >= 4)  // for 循环中找到所有匹配的值,不需要外层for循环在继续
//		{
//			break ;
//		}
		systick_delay_ms(5);
	}
    uart4_dma_tx_send(cmd_buf,6);	  // 发送进入工厂模式的 命令
    for( u8 m = 0 ;m < 100; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 7 ) // 已经进入工厂模式后的返回值有7 个字节
			{
                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
				if(strstr((const char*)char_campare_buf,(const char*)"7F-07-01-80") != NULL) //表示已经进入工厂模式， 收到了：0x7F,0x07,0x01,0x80
                {
                    // 从串口接收到的7个数据中，从第6个开始copy 1个到  factory_mode_flag
                    memcpy(&factory_mode_flag,uart4_copy_ring_buffer + 5,1);
                    if(factory_mode_flag == 0x00)     //   factory_mode_flag == 0, 表示进入到factory mode
                    {
                        memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                        return true ;
                    }         
                }
			}
		}
		systick_delay_ms(5);
	}
    
	// 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf("bl702_factory mode fail，no firmware or no dongle\r\n"); 
	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,360,6,(u8*)"factory mode fail，no firmware or no dongle",Red,Tan); 
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;

}

/*******************************************************************
********************************************************************
* 名称：bool bl702_enter_waterfall_light(void)
* 功能：进入流水模式
* 参数：无
* 返回值：true ： 进入流水灯模式  false ： 进入流水灯模式失败
* 版本：v1
* 作者：matt
* 日期：2023-06-03
* 举例：
* 备注：
* 7F 06 01 13 80 94
* 
*********************************************************************
*********************************************************************/
bool bl702_enter_waterfall_light(void)
{
    u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[6] = {0x7F,0x06,0x01,0x13,0x80,0x94}; //  设备进进入流水io 的命令
    unsigned char waterfall_flag = 0x11;                     //  保存进入工厂模式标志 00 进入工厂，其他失败
	unsigned char_campare_buf[14] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    
    uart4_dma_tx_send(cmd_buf,6);	  // 发送进入流水灯模式的命令
    
    for( u16 m = 0 ;m < 50; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 7 ) // 准备进入工厂模式后的返回值有7个字节
			{
                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
				if(strstr((const char*)char_campare_buf,(const char*)"7F-07-01-93") != NULL) //表示已经接收到返回值， 收到了：0x7F,0x07,0x01,0x93
                {
                    // 从串口接收到的7个数据中，从第6个开始copy 1个到  factory_mode_flag
                    memcpy(&waterfall_flag,uart4_copy_ring_buffer + 5,1);
                    if(waterfall_flag == 0x00)     //   waterfall_flag == 0, 表示进入到流水灯模式
                    {
                        memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                        return true ;
                    }
                }
			}
		}
		systick_delay_ms(10);
	}
   
    // 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf(" bl702 enter waterfall light fail\r\n"); 
	lcd_fill_color(10,388,310,400,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,388,6,(u8*)"bl702 enter waterfall light fail",Red,Tan); 
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
* 名称：bool bl702_get_rssi_value(signed char rssi_value)
* 功能：获取bl702的 rssi值并打印
* 参数：rssi_value ：获取的rssi值，带符号的十进制值
* 返回值：true： 获取成功
         false ：获取失败
* 版本：v1
* 作者：matt
* 日期：2023-06-03
* 举例：
* 备注：
* 
* 发送 7F 06 01 0C 80 8B
  接受 7F 08 01 8C
*********************************************************************
*********************************************************************/
bool bl702_get_rssi_value(signed char rssi_value)
{
    u8 get_rssi_test_times = 0 ;         //  用于控制测试获取rssi值得测试次数
    u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[6] = {0x7F,0x06,0x01,0x0C,0x80,0x8B}; //  设备进入获取RSSI的指令
    unsigned char rssi_flag = 0x11;                     //  保存进入工厂模式标志 00 进入工厂，其他失败
    signed char rssi_value_temp = 0 ;  // 保存获取到的rssi值
	unsigned char_campare_buf[14] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    unsigned  char rssi_string_buf[5] = {0} ;  // 保存rssi的打印字符串值
    uart4_dma_tx_send(cmd_buf,6);	  // 发送获取rssi的指令
    
    for( u8 m = 0 ;m < 200; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 8 ) // 准备进入工厂模式后的返回值有8个字节
			{
                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
				if(strstr((const char*)char_campare_buf,(const char*)"7F-08-01-8C") != NULL) //表示已经接收到返回值， 收到了：0x7F,0x07,0x01,0x8C
                {
//                    usart1_printf("===========\r\n");
//                    usart1_dma_tx_send((uint8_t *)uart4_copy_ring_buffer,8);
//                    usart1_printf("===========\r\n");
                    // 从串口接收到的7个数据中，从第6个开始copy 1个到  rssi_flag
                    memcpy(&rssi_flag,uart4_copy_ring_buffer + 5,1);
                    if(rssi_flag == 0x00)     //   rssi_flag == 0, 表示获取到rssi值
                    {
                        memcpy(&rssi_value_temp,uart4_copy_ring_buffer + 6,1);
                        //========================================================
                        if(rssi_value_temp & 0x80) // 获取到的rssi值 是负数的时候
                        {   
                            rssi_value = (rssi_value_temp & 0x7f) -128 ;
                            if(rssi_value >= BL702_RSII_LEVER)
                            {
                               
                                // 将rssi 的值打印成字符串放在rssi_string_buf
                                sprintf((char *)rssi_string_buf,"%d",rssi_value);                      
                                display_ascii_string(250,258,6,(u8*)rssi_string_buf,Green,Tan); // 显示测试的 rssi 内容  ok
                                
                                sprintf((char *)rssi_string_buf,"[%d]",BL702_RSII_LEVER);  
                                display_ascii_string(280,258,6,(u8*)rssi_string_buf,Black,Tan);         // 显示测试的 rssi 阈值内容  ok
                                
                                memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                                return true ;
                            }
                            else
                            {
                                get_rssi_test_times ++ ;      // 获取rssi 失败的计数 +1 
                                if(get_rssi_test_times < 20)  // 测试rssi 给了 20 次机会
                                {
                                    // 为下一次获取rssi 准备，将相关变量清理干净
                                    memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                                    rssi_value_temp = 0 ;
                                    memset(char_campare_buf,1,sizeof(char_campare_buf));
                                    rssi_flag = 0x11;  
//                                    usart1_printf("get_rssi_test_times = %d\r\n",get_rssi_test_times);
                                    uart4_dma_tx_send(cmd_buf,6);	  // 发送获取rssi的指令
                                }
                                else
                                {
                                    // 将rssi 的值打印成字符串放在rssi_string_buf
                                    sprintf((char *)rssi_string_buf,"%d",rssi_value);                      
                                    display_ascii_string(250,258,6,(u8*)rssi_string_buf,Red,Tan);   // 显示测试的 rssi 内容  ok
                                    
                                    sprintf((char *)rssi_string_buf,"[%d]",BL702_RSII_LEVER);  
                                    display_ascii_string(280,258,6,(u8*)rssi_string_buf,Black,Tan); // 显示测试的 rssi 阈值内容  ok
                                    
                                    break;                          
                                }
                               
                                
                            } 
                        }
                        else  // 获取到的rssi值是正值的时候
                        {
                            rssi_value = rssi_value_temp & 0x7f ;
                            // 将rssi 的值打印成字符串放在rssi_string_buf
                            sprintf((char *)rssi_string_buf,"%d",rssi_value);                      
                            display_ascii_string(250,258,6,(u8*)rssi_string_buf,Green,Tan); // 显示测试的 rssi 内容  ok
                            display_ascii_string(280,258,6,(u8*)"[-60]",Black,Tan);         // 显示测试的 rssi 内容  ok
                            memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                            return true ; 
                        }
                        //========================================================
                    }
                }
			}
		}
		systick_delay_ms(5);
	}
 
    // 能运行到这里来，就表示一定是没有接收到想要的信息
	usart1_printf(" bl702 get RSSI fail\r\n"); 
	lcd_fill_color(10,388,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,388,6,(u8*)"bl702 get RSSI fail",Red,Tan); // 在testlog提示区内，显示测试fail
	
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	lcd_fill_color(153,256,237,273,Red);
	display_ascii_string(189,259,6,(u8*)"NG",Black,Red);   // 显示NG字样
	//==========如果运行到这里，表示芯片从其进入工厂模式失败了==========
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));	
	return false;
}
/*******************************************************************
********************************************************************
* 名称：bool bl702_get_mac(void)
* 功能：702获取 mac信息并打印
* 参数：无
* 返回值： true：   mac读取ok
          false ： mac读取失败
* 版本：v1
* 作者：matt
* 日期：2023-06-03
* 举例：
* 备注：
* 
* 7F 06 01 04 80 83 
*********************************************************************
*********************************************************************/
bool bl702_get_mac(void)
{
    u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
	unsigned char cmd_buf[6] = {0x7F,0x06,0x01,0x04,0x80,0x83}; //  设备获取mac的命令
    unsigned char mac_flag = 0x11;                     //  保存进入工厂模式标志 00 进入工厂，其他失败
	unsigned char_campare_buf[14] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    unsigned char mac_buf[9] = {0} ;  // 保存702读取到的mac地址
    unsigned char mac_hex_buf[30] = {0}; // 保存mac 的 hex值
    memset(bl702_global_mac_hex_buf,0,sizeof(bl702_global_mac_hex_buf)); // 清理bl702_global_mac_hex_buf ，为保存702得到mac地址准备
    
    uart4_dma_tx_send(cmd_buf,6);	  // 发送获取mac的命令
    
    for( u8 m = 0 ;m < 200; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 15 ) // 准备获取到mac后的返回值长度时15个字节
			{
                // 将返回值打印成十六进制的字符串，保存进char_campare_buf  中
				snprintf((char *)char_campare_buf,13,"%02X-%02X-%02X-%02X",uart4_copy_ring_buffer[0],uart4_copy_ring_buffer[1],uart4_copy_ring_buffer[2],uart4_copy_ring_buffer[3]);
				if(strstr((const char*)char_campare_buf,(const char*)"7F-0F-01-84") != NULL) //表示已经接收到返回值， 收到了：0x7F,0x0F,0x01,0x84
                {
                    // 从串口接收到的15个数据中，从第6个开始copy 1个到  mac_flag
                    memcpy(&mac_flag,uart4_copy_ring_buffer + 5,1);
                    if(mac_flag == 0x00)     //   mac_flag == 0, 表示获取到mac
                    {
                        // 从串口接收到的15个数据中，从第7个开始copy 8个到  mac_buf
                        memcpy(mac_buf,uart4_copy_ring_buffer + 6,8); 
						snprintf((char*)bl702_global_mac_hex_buf,16,"%02X%02X%02X%02X%02X%02X%02X%02X",mac_buf[0],mac_buf[1],mac_buf[2],mac_buf[3],mac_buf[4],mac_buf[5],mac_buf[6],mac_buf[7]);
                        snprintf((char*)mac_hex_buf,28,"MAC:%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X",mac_buf[0],mac_buf[1],mac_buf[2],mac_buf[3],mac_buf[4],mac_buf[5],mac_buf[6],mac_buf[7]);
                        usart1_printf("%s\r\n",mac_hex_buf);
                        lcd_fill_color(10,374,312,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
                        display_ascii_string(10,374,6,(u8*)mac_hex_buf,Black,Green); // 在testlog提示区内，显示测试mac 内容
                        
                        
                        memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                        return true ;
                    }
                }
			}
		}
		systick_delay_ms(5);
	}
    
    // 执行到这里的时候，表示没有收到任何信息
	lcd_fill_color(10,374,312,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
	display_ascii_string(10,374,6,(u8*)"bl702 get mac fail",Red,Tan); // 在testlog提示区内，显示测试fail 的mac实测内容
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
* 日期：2023-06-03
* 举例：
* 备注：
* 
* 7F 06 01 01 80 86
*********************************************************************
*********************************************************************/
bool bl702_read_fw_version(void)
{
    u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
    u16 fw_version_msg_length = 0 ;      // 保存固件版本信息的长度 
	unsigned char cmd_buf[6] = {0x7F,0x06,0x01,0x01,0x80,0x86}; //  设备获取固件版本的命令
    unsigned char fw_version_flag = 0x11;                     //  保存进入工厂模式标志 00 进入工厂，其他失败
//	unsigned char_campare_buf[14] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    unsigned char fw_version_buf[100] = {0} ;  // 保存702读取到的固件版本信息
//    unsigned char mac_hex_buf[17] = {0}; // 保存mac 的 hex值
    
    
    uart4_dma_tx_send(cmd_buf,6);	  // 发送获取固件版本的命令
    
    for( u8 m = 0 ;m < 150; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 2 ) // 将获取到的固件版本信息的第2个信息提取出来，表示数据的长度
			{
                
                memcpy(&fw_version_msg_length,uart4_copy_ring_buffer + 1,1);  //  获取到固件版本的返回信息长度
                memcpy(&fw_version_flag,uart4_copy_ring_buffer + 5,1);        //  获取信息是否返回成功，返回成功后，第6个字节数据应该是0
				if(fw_version_flag == 0x00)                                   //  fw_version_flag == 0, 表示获取到固件版本信息
                {
					if(fw_version_msg_length >= 50)  // 用于控制版本信息的长度，超过50个字符，在lcd上显示不了,分成了2行，对于两行的，第2行也会显示不下，暂时不考虑第二行
					{
//						fw_version_msg_length = 50;
						memcpy(fw_version_buf,uart4_copy_ring_buffer + 6,50); // 固件的信息copy  50个字符到 fw_version_buf 中
						lcd_fill_color(10,346,312,358,Tan);                                // 清除掉原先的显示
						display_ascii_string(10,346,6,(u8*)fw_version_buf,Black,Green);
						
						memset(fw_version_buf,0,sizeof(fw_version_buf));
						memcpy(fw_version_buf,uart4_copy_ring_buffer + 6 + 50,fw_version_msg_length - 57); // 固件的的剩余信息 copy到 fw_version_buf 中
						
						lcd_fill_color(10,360,312,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
						display_ascii_string(10,360,6,(u8*)fw_version_buf,Black,Green);
						
						
					}
					else
					{
						memcpy(fw_version_buf,uart4_copy_ring_buffer + 6,fw_version_msg_length - 7); // 固件的信息copy到 fw_version_buf 中
						lcd_fill_color(10,346,312,358,Tan);                                // 清除掉sha256 calculating 位置处的显示
						display_ascii_string(10,346,6,(u8*)fw_version_buf,Black,Green);
					}
                    
					
                    memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                    return true ;
                    
                }
			}
		}
		systick_delay_ms(5);
	}
    
    // 执行到这里的时候，表示没有收到任何信息
	lcd_fill_color(10,346,312,358,Tan);                                // 清除掉sha256 calculating 位置处的显示
	display_ascii_string(10,346,6,(u8*)"bl702 get fw fail",Black,Red);
	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
	return false;
    
}

/*******************************************************************
********************************************************************
* 名称：bool bl702_authorization(void)
* 功能：完成bl702的激活
* 参数：无
* 返回值：true  ：激活成功
         false ：激活失败
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool bl702_authorization(void)
{
	u32 read_num = 0 ;                   // 保存环形队列中有效数据长度
//    u16 fw_version_msg_length = 0 ;      // 保存固件版本信息的长度 
	unsigned char authorization_flag = 0x11;                     //  保存进入工厂模式标志 00 进入工厂，其他失败

//    unsigned char fw_version_buf[50] = {0} ;  // 保存702读取到的固件版本信息
	struct AES_ctx AES_s;                // AES 加密结构通变量
	u8 key[16] = {"soNoFF22soNoFF22"};   // 加密KEY：“soNoFF22soNoFF22”组成的16个字节 
	unsigned char cmd_buf[23] = {0x7F,0x16,0x02,0x00,0x80};    //  设备写入授权码的命令 一共22字节，前5个字节固定，后面+aes加密后的16字节数据
	// 源数据：“SOnoFf22”+ 8个chipID数字字符所组成的16个字节
	unsigned char encrypt_data[16] = {0x53, 0x4F, 0x6E, 0x6F, 0x46, 0x66, 0x32 ,0x32 };  // 等待加密的原数据  SOnoFf22 + chip id
	for(u8 i=8 ; i < 16;i++)
	{
		encrypt_data[i] = bl_702_chip_id[i-8] ;  // 将 chip id 的 0---7 复制到encrypt_data  8---15
	}
	
	AES_init_ctx(&AES_s, key);			    // 初始化 AES
    AES_ECB_encrypt(&AES_s, encrypt_data);	// 加密后的密文覆盖掉原有的 encrypt_data
	
	for(u8 i = 5; i < 21; i++)  //将aes加密的16字节拼装
	{
		cmd_buf[i]  = encrypt_data[i-5] ;
	}

	cmd_buf[21] = xor_fun(cmd_buf, sizeof(cmd_buf)) ; // 异或校验值作为命令的最后一个字节拼装

    uart4_dma_tx_send(cmd_buf,22);	  // 发送激活指令

	
	for( u16 m = 0 ;m < 50; m++ )    // 这个延时很重要，不能太短
	{
		if(UART4_REVEIVE_OK == true)  // 表示串口4收到数据
		{
			UART4_REVEIVE_OK = false ;
			read_num = get_ringbuffer_length_for_usart1(&uart4_receive_buffer) ;
			read_ringbuffer_for_usart1(&uart4_receive_buffer,uart4_copy_ring_buffer,read_num); //将环形队列中的数据读取到一个数组中
			if( read_num >= 7 ) // 将获取到的固件版本信息的第2个信息提取出来，表示数据的长度
			{              
                memcpy(&authorization_flag,uart4_copy_ring_buffer + 5,1);        //  获取信息是否返回成功，返回成功后，第6个字节数据应该是0
				if(authorization_flag == 0x00)                                  //   authorization_flag == 0, 表示授权成功并返回
                {
                    memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
                    return true ;
                    
                }
			}
		}
		systick_delay_ms(10);
	}

	// 程序运行到这里，表示激活失败了
//	lcd_fill_color(10,360,310,372,Tan);  // 清屏上一次显示的烧录相关信息，为接下来的新时间显示做准备
//	lcd_fill_color(153,274,237,291,Red);
//	display_ascii_string(189,277,6,(u8*)"NG",Black,Red);   // 显示ok字样

	memset(uart4_copy_ring_buffer,0,sizeof(uart4_copy_ring_buffer));
    return false ;
	
}


