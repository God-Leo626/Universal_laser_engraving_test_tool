#include "esp8266.h"
#include "stm32f10x.h"
#include "usart_dma_all.h"
#include "systick_delay_freertos.h"
#include "string.h"
#include "usart_ring_buff.h"
//#include "malloc.h"
#include "lcd.h"
#include "w25q32.h"
#include "my_task.h"
//*************************************************************************************
extern char mac_buf[12];
//*************************************************************************************
//连接端口号:8086,可自行修改为其他端口.
const char* portnum = "8080\r\n";		    // 生产服务器端口
const char* ip_address = "52.83.163.118";   // 生产服务器地址
//*************************************************************************************
const char* appid = "t2ikn2s1a4f3y1rszrjjp943f6h633bj";
const char* app_secret = "d191a4a5a6cf68256f2d8b54bf198c33";
//*************************************************************************************
const char* get_fw_version = "/ProductionManagement/firmware/inquire";       // 查询固件
const char* get_down_fw = "/ProductionManagement/firmware/download";         //下载固件
const char* get_license_meg = "/ProductionManagement/license/inquire";       // 查询license 信息
//*************************************************************************************
//WIFI STA模式,设置要去连接的路由器无线参数,请根据你自己的路由器设置,自行修改.
const char* wifista_ssid = "COOLKIT_TEST_STA";			//路由器SSID号  COOLKIT_TEST_STA
const char* wifista_encryption = "wpawpa2_aes";	        //wpa/wpa2 aes加密方式
const char* wifista_password = "12345678";             	//连接密码
//*************************************************************************************
//WIFI AP模式,模块对外的无线参数,可自行修改.
const char* wifiap_ssid = "COOLKIT_TEST_AP";			//对外SSID号
const char* wifiap_encryption = "wpawpa2_aes";       	//wpa/wpa2 aes加密方式
const char* wifiap_password = "12345678"; 		        //连接密码 

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//4个网络模式
//const char* ATK_ESP8266_CWMODE_TBL[3]={"STA_MODE","AP_MODE","AP&STA_MODE"};	//ATK-ESP8266,3种网络模式,默认为路由器(ROUTER)模式 
////4种工作模式
//const char* ATK_ESP8266_WORKMODE_TBL[3]={"TCP_SEVER","TCP_CLIENT"," UDP_MODE"};	//ATK-ESP8266,4种工作模式
////5种加密方式
//const char* ATK_ESP8266_ECN_TBL[5]={"OPEN","WEP","WPA_PSK","WPA2_PSK","WPA_WAP2_PSK"};

//*************************************************************************************
/*
TCP和UDP协议的不同点：

⑴ TCP和UDP都属于socket通信协议，前者是以100个数据流的方式进行通信，后者是以数据包的方式进行通信；

⑵ TCP是有向连接协议，UDP是无向连接协议；

⑶ 当TCP Client和服务器建立连接时，它们需要三个握手协议。UDP不需要握手，直接发送数据包；

⑷ TCP通信不会丢失数据，UDP通信会丢失数据包；

⑸ 在通信可靠性方面，TCP比UDP更可靠；

⑹ 安全性上，TCP安全保密要比UDP高；

⑺ UDP采用的是非阻塞异步通信模式，而TCP是采用的是阻塞异步通信模式
*/
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* pb12
*********************************************************************
*********************************************************************/
void  esp8266_reset_pin_init(void)
{
	//初始化GPIO 的结构体变量
	GPIO_InitTypeDef  GPIO_TypeDef_init;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);  // 使能gpio B口时钟
	//初始化 pin的输入模式，速度，pin选择
	GPIO_TypeDef_init.GPIO_Mode = GPIO_Mode_Out_PP;          // 推挽输出
	GPIO_TypeDef_init.GPIO_Pin = GPIO_Pin_12;
	GPIO_TypeDef_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_TypeDef_init);
	
}
/*******************************************************************
********************************************************************
* 名称：void esp8266_reset_ctrl(u8 time)
* 功能： 复位wifi模块
* 参数： time: ms
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void esp8266_reset_ctrl(u16 timer)
{
	WIFI_RESET = 0;
	systick_delay_ms(timer);
	WIFI_RESET = 1;
}

/*******************************************************************
********************************************************************
* 名称：void esp_8266_at_response(u8 mode)
* 功能：返回串口3接收到字符
* 参数：mode : 1 :删除接收到数据的标志位  0： 不删除标准位
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
void esp_8266_at_response(u8 mode)
{
	unsigned int receive_len = 0;
	char copy_ring_buffer[USART3_REC_LEN] = {0};
	if(USART3_REVEIVE_OK)		//接收到一次数据了
	{ 
		receive_len = get_ringbuffer_length(&usart3_receive_buffer);	      // 获取环形队列中的有效数据	
		read_ringbuffer(&usart3_receive_buffer,copy_ring_buffer,receive_len); //将环形队列中的数据读取到一个数组中
		usart1_printf("%s",copy_ring_buffer);	//发送到串口1
		if(mode)
		{
			USART3_REVEIVE_OK = 0;
		}
	} 
}
/*******************************************************************
********************************************************************
* 名称：u8* esp_8266_check_cmd(char *str)
* 功能：判断一个命令/数据 发送后是否有出现期望的返回字符
* 参数：希望返回的字符
* 返回值： 0： 没有期望的字符出现   其他：期望的字符
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
u8* esp_8266_check_cmd(char *str)
{
	unsigned int receive_len = 0;
	char copy_ring_buffer[RING_BUFFER_LEN] ={0};
	char *strx = 0;
	if(USART3_REVEIVE_OK)		//接收到一次数据了
	{ 
		receive_len = get_ringbuffer_length(&usart3_receive_buffer);	      // 获取环形队列中的有效数据	
		read_ringbuffer(&usart3_receive_buffer,copy_ring_buffer,receive_len); //将环形队列中的数据读取到一个数组中
		strx = strstr((const char*)copy_ring_buffer,(const char*)str);        // 比较接收到的数据中，有无希望的字符出现，并返回该字符
	} 
	return (u8*)strx;
}
/*******************************************************************
********************************************************************
* 名称：bool esp_8266_send_cmd(char *cmd,char *ack,u8 times,u16 waittime)
* 功能：向ESP8266发送命令，并等待应答，应答时间= 次数x 每次的时间
       发送成功，返回应答值   失败：无应答值
* 参数： cmd：发送的命令 
        ack：期待的应答字符
		times ：命令发送后，设置等待的次数
		waittime ：每次等待的时间 (单位ms)
* 返回值： true:命令发送成功且有应答   false： 命令发送失败
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 


*********************************************************************
*********************************************************************/
bool esp_8266_send_cmd(char *cmd,char *ack,u8 times,u16 waittime)
{
	usart3_printf("%s\r\n",cmd);	//要发送的命令
	if(ack && times)		    //判断是否需要等待应答（有些命令不需要应答）
	{
		while(times--)	        //等待的次数
		{
			systick_delay_ms(waittime);//单次等待的时间
			if(USART3_REVEIVE_OK)      //接收到了数据
			{
				if(esp_8266_check_cmd(ack) != 0) // 判断此次接收到的数据是否有期望的字符 （0：没有  其他：有）
				{
					usart1_printf("[USART3_SEND_CMD]: %s -->[ESP8266 ACK]: %s\r\n",cmd,(u8*)ack);
					USART3_REVEIVE_OK = false;  // 清零接收标志
					return true ;
				}
				USART3_REVEIVE_OK = false;  // 清零接收标志
			} 
		}
		return false ;
		
	}//当不需要返回值的时候，就直接返回真
	return true ;
} 
bool esp_8266_send_cmd_back_passthrough(char *cmd,char *ack,u8 times,u16 waittime)
{
	u3_printf("%s",cmd);	//要发送的命令
	if(ack && times)		    //判断是否需要等待应答（有些命令不需要应答）
	{
		while(times--)	        //等待的次数
		{
			systick_delay_ms(waittime);//单次等待的时间
			if(USART3_REVEIVE_OK)      //接收到了数据
			{
				if(esp_8266_check_cmd(ack) != 0) // 判断此次接收到的数据是否有期望的字符 （0：没有  其他：有）
				{
					usart1_printf("[CMD]: %s -->[ACK]: %s\r\n",cmd,(u8*)ack);
					return true ;
				}
				USART3_REVEIVE_OK = false;  // 清零接收标志
			} 
		}
		return false ;
		
	}//当不需要返回值的时候，就直接返回真
	return true ;
}
/*******************************************************************
********************************************************************
* 名称：bool esp_8266_send_data(char *data,char *ack,u8 times,u16 waittime)
* 功能：向ESP8266发送数据，并等待应答，应答时间= 次数x 每次的时间
       发送成功，返回应答值   失败：无应答值
* 参数： data：发送的数据
        ack：期待的应答字符
		times ：命令发送后，设置等待的次数
		waittime ：每次等待的时间 (单位ms)
* 返回值： true:数据发送成功且有应答   false： 数据发送失败
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
*
* 
* 
*********************************************************************
*********************************************************************/
bool esp_8266_send_data(char *data,char *ack,u8 times,u16 waittime)
{
	u3_printf("%s",data);	    //要发送的数据，不带 \r\n
	if(ack && times)		    //判断是否需要等待应答（有些命令不需要应答）
	{
		while(times--)	        //等待的次数
		{
			systick_delay_ms(waittime);//单次等待的时间
			if(USART3_REVEIVE_OK)      //接收到了数据
			{
				if(esp_8266_check_cmd(ack) != 0) // 判断此次接收到的数据是否有期望的字符 （0：没有  其他：有）
				{
					if(esp_8266_check_cmd(ack))
					{
						return true ;
					}
				}
				USART3_REVEIVE_OK = false;  // 清零接收标志
			} 
		}
		return false ;
		
	}//当不需要返回值的时候，就直接返回真
	return true ;
}
/*******************************************************************
********************************************************************
* 名称：bool esp_8266_quit_passthrough (void)
* 功能：ESP8266退出透传模式
* 参数：
* 返回值： true : 退出透传ok   false：退出透传失败
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void esp_8266_quit_passthrough (void)
{
//	bool result = false ;
//	while((USART3->SR&0X40)==0);	//等待发送空
//	USART3->DR='+';      
////	systick_delay_ms(30);		    //大于串口组帧时间(10ms)
//	while((USART3->SR&0X40)==0);	//等待发送空
//	USART3->DR='+';      
////	systick_delay_ms(30);			//大于串口组帧时间(10ms)
//	while((USART3->SR&0X40)==0);	//等待发送空
//	USART3->DR='+';      
//	systick_delay_ms(300);					//等待500ms
	usart3_printf("+++");
	systick_delay_ms(1000);	
//	while(esp_8266_send_cmd("AT+CIPMODE=0","OK",5,20) == false);//发送“AT+CIPMODE=0”来关闭透传模式，这样模块重启之后就不会进入 AT 模式了
	while(esp_8266_send_cmd("AT","OK",5,20) == false);
    
		
	
	 
}

//退出透传模式
//u8 esp8266_quit_trans(void)
//{
//	u8 result=0;
//	usart3_printf("+++");
//	systick_delay_ms(1000);					
//	result=esp_8266_send_cmd("AT","OK",5,20);
//	if(result)
//		printf("quit_trans success!");
//	else	
//		printf("quit_trans failed!");
//	return result;
//}

/*******************************************************************
********************************************************************
* 名称：bool esp_8266_apsta_check(void)
* 功能：获取ATK-ESP8266模块的AP+STA连接状态
* 参数： 无
* 返回值： true ： 连接ok  false ：未连接
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool esp_8266_apsta_check(void)
{
	esp_8266_quit_passthrough();                //退出透传
	esp_8266_send_cmd("AT+CIPSTATUS",":",4,10); //发送AT+CIPSTATUS指令,查询连接状态
	if( esp_8266_check_cmd("+CIPSTATUS:0")&&
		esp_8266_check_cmd("+CIPSTATUS:1")&&
		esp_8266_check_cmd("+CIPSTATUS:2")&&
		esp_8266_check_cmd("+CIPSTATUS:4"))
	{
		return false;
	}
	else
	{
		return true;
	}	
}
/*******************************************************************
********************************************************************
* 名称：bool esp_8266_connect_status_check(void)
* 功能：获取ATK-ESP8266模块的连接状态
* 参数：
* 返回值：false :未连接   true: 连接ok
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool esp_8266_connect_status_check(void)
{
//	u8 *p;
//	if(esp_8266_quit_passthrough() == false) //退出透传失败
//	{
//		return false;
//	}
	 return esp_8266_send_cmd("AT+CIPSTATUS","+CIPSTATUS:",4,20); //发送AT+CIPSTATUS指令,查询连接状态
//	p = esp_8266_check_cmd("+CIPSTATUS:"); 
//	res = *p;									//得到连接状态	

	
}
/*******************************************************************
********************************************************************
* 名称：void esp_8266_get_wanip(u8* ipbuf)
* 功能：获取 STA 或者 AP 模式下的 ip 地址
* 参数： ip地址输出缓存区
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
void esp_8266_get_wanip(u8* ipbuf)
{
	u8 *p,*p1;
	if(esp_8266_send_cmd("AT+CIFSR","OK",4,10) == false ) //获取WAN IP地址失败
	{
		ipbuf[0]=0;
		return;
	}
	p = esp_8266_check_cmd("\"");
	p1 = (u8*)strstr((const char*)(p+1),"\"");
	*p1 = 0;
	sprintf((char*)ipbuf,"%s",p+1);	

}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
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
void esp8266_reconnection_set(void)
{
	char p[40] = {0} ;

	esp_8266_quit_passthrough ();                                      // 死等退出透传模式（才能在AT模式下发送命令）
//	esp_8266_send_cmd("AT+RST","OK",5,100);            // 死等 重启wifi模块
//	systick_delay_ms(4000);
//	while(esp_8266_send_cmd("AT+CIPMUX=0","OK",4,10) == false);                   // 2开启单连接  0：单连接   1：多连接
    sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",wifista_ssid,wifista_password);     // 设置8266要连接的无线参数:ssid,密码
    while(esp_8266_send_cmd((char *)p,"WIFI GOT IP",4,1000) == false);		      // 连接目标路由器,并且获得IP
//	systick_delay_ms(4000);
	
//	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ip_address,(u8*)portnum);    // 配置目标TCP服务器    例子： AT+CIPSTART="TCP","192.168.1.XXX",8086
//	while(esp_8266_send_cmd((char*)p,"OK",5,50)== false);                         // 死等 连接目标服务器

//	while(esp_8266_send_cmd("AT+CIPMODE=1","OK",4,10) == false);                  // 设置透传	                       
//	while(esp_8266_send_cmd("AT+CIPSEND","OK",8,10) == false);                    // 启动发送
	display_ascii_string(152,108,8,(u8*)wifista_ssid,Green,Tan);                   // WIFI SSID
	display_ascii_string(152,126,8,(u8*)wifista_password,Green,Tan);               // wifi 密码
}






/*******************************************************************
********************************************************************
* 名称： bool esp8266_sta_mode_set_init(void)
* 功能： esp8266 初始化为 sta 模式
* 参数：
* 返回值： true : 初始化ok
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void esp8266_sta_mode_set_init(void)
{
	    char p[40] = {0} ; 
		
		while(esp_8266_send_cmd("AT+RST","OK",50,15) == false);            // 死等 重启wifi模块
//		systick_delay_ms(1000);                                            // 等待重启ok

		while(esp_8266_send_cmd("ATE0","OK",4,10) == false);              // 死等 关闭回显
		while(esp_8266_send_cmd("AT+CWMODE_DEF=1","OK",4,10) == false);       // 配置WiFi模组工作模式为单STA模式，并把配置保存在flash    
		while(esp_8266_send_cmd("AT+CIPMUX=0","OK",4,10) == false);                   // 2开启单连接  0：单连接   1：多连接

		while(esp_8266_send_cmd("AT+CWHOSTNAME=\"COOLKIT_TEST_BOARD_WIFI\"","OK",4,10) == false);     // 设置sta 主机名称 主机名称，最长 32 字节
		sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",wifista_ssid,wifista_password);     // 设置8266要连接的无线参数:ssid,密码
		while(esp_8266_send_cmd((char *)p,"WIFI GOT IP",4,600) == false);		      // 连接目标路由器,并且获得IP

		while(esp_8266_send_cmd("AT+CWAUTOCONN=1","OK",4,10) == false);               // 使能上电自动连接AP
//		while(esp_8266_send_cmd("AT+UART_DEF=921600,8,1,0,0","OK",4,10) == false);    // 将wifi模块的波特率设置为921600

		
//		sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ip_address,(u8*)portnum);    // 配置目标TCP服务器    例子： AT+CIPSTART="TCP","192.168.1.XXX",8086
//		while(esp_8266_send_cmd((char*)p,"OK",5,50)== false);                         // 死等 连接目标服务器
		
//		while(esp_8266_send_cmd("AT+CIPMODE=1","OK",4,10) == false);                  // 设置透传	                       
//		while(esp_8266_send_cmd("AT+CIPSEND","OK",4,10) == false);                    // 启动发送
//		systick_delay_ms(10);  
		//**************************************************************************************
		//程序运行到这里的时候，表示网络连接正常，可以显示ssid 和  password
//		display_ascii_string(153,36,8,(u8*)wifista_ssid,Green,Tan);                   // lcd 上显示WIFI SSID
//		display_ascii_string(153,54,8,(u8*)wifista_password,Green,Tan);               // lcd 上显示wifi 密码
//		//**************************************************************************************
//		usart1_printf("ESP8266 STA MODE SET OK\r\n");

}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
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
void esp8266_sta_coolkit_server_connect(void)
{
	char p[400] = {0} ;

	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",ip_address,(u8*)portnum);    // 配置目标TCP服务器    例子： AT+CIPSTART="TCP","192.168.1.XXX",8086
	                                                                                               // https://test-pdmg.sonoff.co
	while(esp_8266_send_cmd((char*)p,"OK",5,100)== false);                         // 死等 连接目标服务器  AT+CIPSEND=46
	
	while(esp_8266_send_cmd("AT+CIPMODE=1","OK",4,10) == false);                  // 设置透传	                       
	while(esp_8266_send_cmd("AT+CIPSEND","OK",8,10) == false);                    // 启动发送
	usart1_printf("COOLKIT SERVER CONNECT [== OK ==]\r\n"); 

	

}

/*******************************************************************
********************************************************************
* 名称：uint32_t esp8266_sta_http_json_tx(char* http_request,char* http_mode,char* url)
* 功能：拼接要向服务器发送的字符串
* 参数：http_request：保存拼接好的字符串
http_mode ：请求模式  GET  POST
url : 要访问的网址
need_path :是否需要拼接路径 0：不需要 1： 需要
burn_start ：在需要拼接路径时，指定的开始下载的固件的起始位置
burn_end ：  在需要拼接路径时，指定的开始下载的固件的终止位置
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：
* 
* // 拼接要处理的http 请求行，请求报头，空行信息
*********************************************************************
*********************************************************************/
uint32_t esp8266_sta_http_json_tx(char* http_request,char* http_mode,char* url,bool need_path,u32 burn_start,u32 burn_end)
{
	// 注意 http_request 的大小
	char request_data[400];                                                 // 所有组装拼接的字符都放在这个字符数组里
	char Sequence_buf[22]= {0};
	u8 token[33] = {0}; // 保存读取到的token,token为32字节，空间要大1=33
	memset(request_data, 0, 30);
	memset(http_request, 0, 100);
	
	// X-CK-Appid
	strcat(request_data, "X-CK_Appid:");                                    // 固定
	strcat(request_data, " t2ikn2s1a4f3y1rszrjjp943f6h633bj\r\n"); 
	//Authorization
	strcat(request_data, "Authorization:");                                 // API调用凭证（如果凭证是计算得到的签名，则该参数为"Sign " + 签名，如果凭证是access_token, 则该参数为"Bearer " + access_token ）
	
//	strcat(request_data, " Bearer 74c17979cb2e229829a7400d48f2ae78\r\n");   // 设备唯一编码 （下位机上传 mac ，chip id，工单配置上位机计算后下发 设备唯一编码token）
	strcat(request_data," Bearer ");
	w25q32_flash_read((u8*)token,0x00000000,32,2);        // 从芯片2里的 0x00地址读取 token，保存在token
//	usart1_printf("token====:%s",token);
	strcat(request_data,(const char *)token);
	strcat(request_data,"\r\n");
	//Content-Type
//	strcat(request_data, "Content-Type:");           // 默认为 application/json；对于文件上传类请求为Multipart/form-data
//	strcat(request_data, " application/json\r\n");
	
	//X-CK-Sequence
	strcat(request_data, "X-CK-Sequence:");          // 长度 13的大小写字母和数字，客户端应尽量使用随机字符串  mac_buf
	snprintf(Sequence_buf,14,"%s%s"," ",mac_buf);    // 将mac地址作为参数发送，截取13个字符 （空格 + 12mac）
	strcat(request_data,Sequence_buf) ;
	strcat(request_data,"\r\n") ;
 
	
	//X-CK-Request-Object
	strcat(request_data, "X-CK-Request-Object:");   // 用于区分请求对象是用户还是设备。参数：user-使用账号密码登录的对象，device-由用户授权的设备
	strcat(request_data, " device");

	
	
	// 拼装 向服务器发送GET请求的http请求行，请求头，空行
	strcat(http_request, http_mode);                   // GET , 或者POST 
	strcat(http_request, url);                         // URL
	if(need_path == true) // 当需要拼接path路径时
	{
		strcat(http_request,path_global_buf);
	}
	strcat(http_request, " HTTP/1.1\r\n");             // 使用1.1版本HTTP
	strcat(http_request, "Host: ");                    // 拼接 Host: 
	strcat(http_request, ip_address);                  // 网址 "https://test-pdmg.sonoff.co"
	strcat(http_request,":");
	strcat(http_request, portnum);                     // 端口 ":8080\r\n"
	if(need_path == true)                              // 当需要拼接固件下载区间时
	{
		strcat(http_request,"Range: bytes=");          // 拼接Range: bytes=
		memset(Sequence_buf,0,22);
//		usart1_printf("+++%d\r\n",sprintf(Sequence_buf,"%d-%d",burn_start,burn_end));  // sprintf的返回值是字符数组中字符的个数
		snprintf(Sequence_buf,sprintf(Sequence_buf,"%u-%u",burn_start,burn_end)+1,"%u-%u",burn_start,burn_end);    //格式化后的字符串长度 < size，则将此字符串全部复制到str中，并给其后添加一个字符串结束符('\0')
//		usart1_printf("Sequence_buf ==%s\r\n",Sequence_buf);
		strcat(http_request,Sequence_buf);
		strcat(http_request,"\r\n");
	
	}
//	strcat(http_request, "Content-Type: application/json\r\n");
    //	strcat(http_request, "Connection: Keep-Alive\r\n");  // 保持连接
//	strcat(http_request, "Content-Length:17\r\n");           //要请求要发送的数据长度
//	strcat(http_request, "Cache-Control: no-cache\r\n");     // 不使用缓存
	// 将 请求行， 请求报头，空行，请求数据合并
	strcat(http_request, request_data);
	strcat(http_request, "\r\n\r\n\r\n");
	
	return strlen(http_request);
}

