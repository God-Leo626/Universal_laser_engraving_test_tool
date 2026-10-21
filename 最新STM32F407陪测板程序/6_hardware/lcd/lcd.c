#include "lcd.h"
//#include "systick_delay.h"
#include "systick_delay_freertos.h"
#include "stdbool.h"
#include "coolkit_logo.h"
#include "font.h"

u8 *  fontbuf; //字库缓存
//===========================================================================
/*******************************************************************
********************************************************************
* 名称： void lcd_pin_init(void)
* 功能： 初始化和lcd显示控制的相关gpio
* 参数： null
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
 static void lcd_pin_init(void)
{
		// 定义 gpio结构体变量，用来初始化GPIO
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能使用 fsmc的 gpio时钟，100pin的 fsmc 使用了 GPIOD GPIOE
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |RCC_APB2Periph_AFIO, ENABLE);
//	  // 使能fmsc 时钟
//    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
//		// 使能复用功能时钟
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO ,ENABLE);
	
		// LCD 的背光控制pin  PD12: BL(L:亮    H:灭) 
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
		
	  // LCD 的复位控制pin   PD13:RESET（L:复位  H：正常工作）
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
	 		   
    // 配置 fsmc 的数据引脚FSMC D0-D15  PD 14 15 0 1  PE 7 8 9 10 11 12 13 14 15  PD 8 9 10

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | 
                                  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
		
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | 
                                  GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure); 
    
		
		
		// 初始化lcd 的 RD  WR  CS  RS 引脚
	  // PD4:  FSMC_NOE  RD   
	  // PD5:  FSMC_NWE  WR  
	  // PD7:  FSMC_NE1  CS   
	  // PD11: FSMC A16  RS 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);  
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 ; 
    GPIO_Init(GPIOD, &GPIO_InitStructure);  
    
    /* tft control gpio init */	 
		// 设置 RD  WR  CS  RS 的默认值
    GPIO_SetBits(GPIOD, GPIO_Pin_13);		 // RST = 1 
    GPIO_SetBits(GPIOD, GPIO_Pin_4);		 // RD  = 1  
    GPIO_SetBits(GPIOD, GPIO_Pin_5);		 // WR  = 1 
    GPIO_SetBits(GPIOD, GPIO_Pin_7);		 //	CS  = 1 
    GPIO_SetBits(GPIOD, GPIO_Pin_12);		 //	BL  = 1  灭
	}

	/*******************************************************************
  ********************************************************************
  * 名称：  static void lcd_fsmc_init(void)
  * 功能：
  * 参数：
  * 返回值：
  * 版本：v1
  * 作者：matt
  * 日期：2021-11-02
  * 备注：
  * 
  * 
  *********************************************************************
  *********************************************************************/
 static void lcd_fsmc_init(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  fsmc_lcd; 
    
    // 使能fmsc 时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	
    fsmc_lcd.FSMC_AddressSetupTime       = 0x00;	 //地址建立时间
    fsmc_lcd.FSMC_AddressHoldTime        = 0x00;	 //地址保持时间
    fsmc_lcd.FSMC_DataSetupTime          = 0x01;		 //数据建立时间
    fsmc_lcd.FSMC_BusTurnAroundDuration  = 0x00;
    fsmc_lcd.FSMC_CLKDivision            = 0x00;
    fsmc_lcd.FSMC_DataLatency            = 0x00;
		//norflash一般用模式B，SRAM一般用模式A，和时序有关
    fsmc_lcd.FSMC_AccessMode             = FSMC_AccessMode_B;	 // 一般使用模式B来控制LCD
    
    FSMC_NORSRAMInitStructure.FSMC_Bank                  = FSMC_Bank1_NORSRAM1;  //使用NE1
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux        = FSMC_DataAddressMux_Disable; //不复用数据地址
    FSMC_NORSRAMInitStructure.FSMC_MemoryType            = FSMC_MemoryType_NOR;   //设置为NOR类型
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth       = FSMC_MemoryDataWidth_16b;  //存储器数据宽度为16bit
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode       = FSMC_BurstAccessMode_Disable;  //
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity    = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode              = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive      = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation        = FSMC_WriteOperation_Enable;   //寄存器写使能
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal            = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode          = FSMC_ExtendedMode_Disable;    //扩展位不使能，读写使用相同时序
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst            = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &fsmc_lcd;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &fsmc_lcd; 
    
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure); 
    
    /* Enable FSMC Bank1_SRAM Bank */
    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  //使能BNAK1，NE1
		
//	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
//  FSMC_NORSRAMTimingInitTypeDef  readWriteTiming; 
//	FSMC_NORSRAMTimingInitTypeDef  writeTiming;
// 
//	readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns
//  readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到	
//  readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // 数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快，尤其对1289这个IC。
//  readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
//  readWriteTiming.FSMC_CLKDivision = 0x00;
//  readWriteTiming.FSMC_DataLatency = 0x00;
//  readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 
//    

//	writeTiming.FSMC_AddressSetupTime = 0x00;	 //地址建立时间（ADDSET）为1个HCLK  
//  writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A		
//  writeTiming.FSMC_DataSetupTime = 0x03;		 //数据保存时间为4个HCLK	
//  writeTiming.FSMC_BusTurnAroundDuration = 0x00;
//  writeTiming.FSMC_CLKDivision = 0x00;
//  writeTiming.FSMC_DataLatency = 0x00;
//  writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A 

// 
//  FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;//  这里我们使用NE1 ，
//  FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
//  FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
//  FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit   
//  FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
//  FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
//	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable; 
//  FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
//  FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
//  FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  存储器写使能
//  FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;   
//  FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
//  FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable; 
//  FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
//  FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序

//  FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

// 	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);  //使能BNAK1，NE1

}
/*******************************************************************
********************************************************************
* 名称：void lcd_write_addr(u16 reg )
* 功能：发送命令给lcd
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void lcd_write_addr(u16 reg )
{
	*(__IO uint16_t *)(BANK1_LCD_CMD)  = reg ;   // 发送地址
}

/*******************************************************************
********************************************************************
* 名称：void lcd_write_data(u16 data )
* 功能：发送数据给lcd
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void lcd_write_data(u16 value )
{
	*(__IO uint16_t *)(BANK1_LCD_DATA) = value;  // 发送数据
}


/*******************************************************************
********************************************************************
* 名称：u16 lcd_read_register(u8 register)
* 功能：
* 参数：u8 regist：要读取的寄存器地址 
* 返回值：返回此寄存器内的值
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static u16 lcd_read_register(void)
{
	return *(BANK1_LCD_DATA);
}

/*******************************************************************
********************************************************************
* 名称： void lcd__write_register(u16 cmd,u16 value)
* 功能： lcd写命令
* 参数：u16 cmd：寄存器地址  ,u16 value：写入此寄存器的数据
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd__write_register(u16 cmd,u16 value)
{
	*(BANK1_LCD_CMD)  = cmd ;   // 发送命令
	*(BANK1_LCD_DATA) = value;  // 发送数据
}




/*******************************************************************
********************************************************************
* 名称： void lcd_bl_ctrl(bool on_off)
* 功能：lcd背光控制开关
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_bl_ctrl(bool on_off)
{
	if(on_off == true)
	{
		LCD_BL = 0;  // 背光亮
	}
	else
	{
		LCD_BL = 1;  // 背光灭
	}
}

/*******************************************************************
********************************************************************
* 名称：void lcd_ili9488_set_windows(u16 x_start,u16 y_start,u16 x_end,u16 y_end)
* 功能：设置lcd的显示区域  
* 参数：x_start，y_start 窗口左上角坐标  x_end   y_end， 窗口右下角坐标
* 返回值： null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_ili9488_set_windows(u16 x_start,u16 y_start,u16 x_end,u16 y_end)
{
	uint8_t x_temp = 0,y_temp = 0;
	lcd_write_addr(0x2A);
	x_temp = (x_start >> 8) & 0x00FF;
	lcd_write_data(x_temp);	
  x_temp = x_start & 0x00FF;
	lcd_write_data(x_temp);
	
	x_temp = (x_end >> 8) & 0x00FF;
	lcd_write_data(x_temp);	
  x_temp = x_end & 0x00FF;
	lcd_write_data(x_temp);


	lcd_write_addr(0x2B);	
	y_temp = (y_start >> 8) & 0x00FF;
	lcd_write_data(y_temp);	
  y_temp = y_start & 0x00FF;
	lcd_write_data(y_temp);
	
	y_temp = (y_end >> 8) & 0x00FF;
	lcd_write_data(y_temp);	
  y_temp = y_end & 0x00FF;
	lcd_write_data(y_temp);	

}





/*******************************************************************
********************************************************************
* 名称：  void lcd_ili9488_init(void)
* 功能： lcd寄存器系列初始化
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void lcd_ili9488_register_init(void)
{
	//************* Start Initial Sequence **********//
	lcd_write_addr(0xE0);
  lcd_write_data(0x00);
	lcd_write_data(0x13);
	lcd_write_data(0x18);   //0f
	lcd_write_data(0x04);   //0d
	lcd_write_data(0X0F);   //1b
	lcd_write_data(0x06);   //0a
	lcd_write_data(0x3A);   //3c
	lcd_write_data(0x56);    //78
	lcd_write_data(0x4D);    //4a
	lcd_write_data(0X03);    //07
	lcd_write_data(0x0A);    //0e
	lcd_write_data(0x06);     //09
	lcd_write_data(0x30);     //1b
	lcd_write_data(0x3E);   
	lcd_write_data(0X0f);
	/* NGAMCTRL (Negative Gamma Control) (E1h)  */	
	lcd_write_addr(0xE1);
	lcd_write_data(0x00);
	lcd_write_data(0x13);     //22
	lcd_write_data(0x18);     //24
	lcd_write_data(0x01);     //06
	lcd_write_data(0X11);     //12
	lcd_write_data(0x06);     //07
	lcd_write_data(0x38);     //36
	lcd_write_data(0x34);     //47
	lcd_write_data(0x4D);     //47
	lcd_write_data(0X06);     //06
	lcd_write_data(0x0D);     //0a
	lcd_write_data(0x0B);     //07
	lcd_write_data(0x31);     //30
	lcd_write_data(0x37);
	lcd_write_data(0X0f);

	/* Power Control 1 (C0h)  */
	lcd_write_addr(0XC0);
	lcd_write_data(0x18);     //10
	lcd_write_data(0x17);     //10
	
	 /* Power Control 2 (C1h) */
  lcd_write_addr(0xC1);
	lcd_write_data(0x41);  //VGH,VGL
	
	 /* VCOM Control (C5h)  */
	lcd_write_addr(0XC5);
	lcd_write_data(0x00);
	lcd_write_data(0x1a);    //22
	lcd_write_data(0x80);    //80
	
	  /* Interface Pixel Format (3Ah) */
  lcd_write_addr(0x3A);
  lcd_write_data(0x55);/* 0x55 : 16 bits/pixel  */
	
	lcd_write_addr(0XB0);   // Interface Mode Control
  lcd_write_data(0x00);   
	
	lcd_write_addr(0xB1);   //Frame rate
  lcd_write_data(0xA0);   //60Hz
	
		/* Display Inversion Control (B4h) */
	lcd_write_addr(0xB4);    
	lcd_write_data(0x02);
	
		/* Display Function Control (B6h)  */
	lcd_write_addr(0xB6);     //RGB/MCU Interface Control
	lcd_write_data(0x02);     //MCU RGB
	lcd_write_data(0x02);     //Source,Gate scan dieection

	/* Set Image Function (E9h)  */
  lcd_write_addr(0xE9);    // Set Image Function
	lcd_write_data(0x00);   //disable 24 bit data input

	
  /* Adjust Control 3 (F7h)  */
  lcd_write_addr(0XF7);
  lcd_write_data(0xA9);
  lcd_write_data(0x51);
  lcd_write_data(0x2C);
  lcd_write_data(0x82);/* DSI write DCS command, use loose packet RGB 666 */
	
	lcd_write_addr(0x21);  //Normal Black
	 /* Sleep Out (11h) */
	lcd_write_addr(0x11);  
  systick_delay_us(60);
	lcd_write_addr(0x28);  

/* 设置屏幕方向和尺寸 */
	lcd_set_direction(LCD_DIRECTION);// 2 横屏  3： 竖屏
	systick_delay_ms(10);

	/* Display On */ 
	lcd_write_addr(0x29); 
  lcd_ili9488_set_windows(0,0,LCD_DEFAULT_WIDTH-1,LCD_DEFAULT_HEIGTH-1);
}

/*******************************************************************
********************************************************************
* 名称： void lcd_set_direction( uint8_t mode_select )
* 功能：设置lcd的扫描方式
* 参数： mode_select：取值1---8    2横屏 3：竖屏
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
  * 返 回 值: 无
  * 说    明：无
  
* 
*********************************************************************
*********************************************************************/
void lcd_set_direction( uint8_t mode_select )
{	
/**
  * Memory Access Control (36h)
  * This command defines read/write scanning direction of the frame memory.
  *
  * These 3 bits control the direction from the MPU to memory write/read.
  *
  * Bit  Symbol  Name  Description
  * D7   MY  Row Address Order     -- 以X轴镜像
  * D6   MX  Column Address Order  -- 以Y轴镜像
  * D5   MV  Row/Column Exchange   -- X轴与Y轴交换
  * D4   ML  Vertical Refresh Order  LCD vertical refresh direction control. 
  *
  * D3   BGR RGB-BGR Order   Color selector switch control
  *      (0 = RGB color filter panel, 1 = BGR color filter panel )
  * D2   MH  Horizontal Refresh ORDER  LCD horizontal refreshing direction control.
  * D1   X   Reserved  Reserved
  * D0   X   Reserved  Reserved
  */
	switch ( mode_select )
	{
		case 1:
/*
		x(319)<--------(0,0)
						       |
						       |
									 |
									 |
									 |
		               |
		       ==fpc== V
						       y(479)
		
*/
			lcd_write_addr(0x36); 
			lcd_write_data((0<<7)|(0<<6)|(0<<5)|(1<<3));     // ,MY==0,MX==0,MV==0 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;
		

		case 2:
 /*
		  (0,0)-------------------->x(479)
					|                    ||
					|                    fpc
					|                    ||
					|
		      V
					y(319)
		
*/
			lcd_write_addr(0x36); 
			lcd_write_data((0<<7)|(0<<6)|(1<<5)|(1<<3));     // ,MY==0,MX==0,MV==1 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;
		
		case 3:
/*
		  (0,0)------->x(319)
					|
					|
					|
					|
		      V ==fpc==
					y(479)	
*/
			lcd_write_addr(0x36); 
			lcd_write_data((0<<7)|(1<<6)|(0<<5)|(1<<3));     // ,MY==0,MX==1,MV==0 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;

		case 4:
 /*
		  (0,0)------->y(319)
					|
					|
					|
					|
		      V ==fpc==
					x(479)
		
*/
			lcd_write_addr(0x36); 
			lcd_write_data((0<<7)|(1<<6)|(1<<5)|(1<<3));     // ,MY==0,MX==1,MV==1 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;
			
		case 5:
 /*
		              y(479)
		              /\
					        |
					        |
					        |
					        |
	x(319) <-------(0,0)
		      ==fpc==
		
		
*/
			lcd_write_addr(0x36); 
			lcd_write_data((1<<7)|(0<<6)|(0<<5)|(1<<3));     // ,MY==1,MX==0,MV==0 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;
			
		case 6:
 /*
		              x(479)
		              /\
					        |
					        |
					        |
					        |
	y(319) <-------(0,0)
		      ==fpc==
		
		
*/
			lcd_write_addr(0x36); 
			lcd_write_data((1<<7)|(0<<6)|(1<<5)|(1<<3));     // ,MY==1,MX==0,MV==1 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;
			
		case 7:
 /*
		     y(479)
		    /\
				|
				|
				|
				|
	(0,0) -------> x(319)
		     ==fpc==
		
		
*/
			lcd_write_addr(0x36); 
			lcd_write_data((1<<7)|(1<<6)|(0<<5)|(1<<3));     // ,MY==1,MX==1,MV==0 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;
			
		case 8:
 /*
		     x(479)
		    /\
				|
				|
				|
				|
	(0,0) -------> y(319)
		     ==fpc==
		
		
*/
			lcd_write_addr(0x36); 
			lcd_write_data((1<<7)|(1<<6)|(1<<5)|(1<<3));     // ,MY==1,MX==1,MV==1 BGR==1
		
			lcd_write_addr(0x2A); 
			lcd_write_data(0x00);	                           /* x start */	
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_WIDTH-1)>>8);        /* x end */	
			lcd_write_data((LCD_DEFAULT_WIDTH-1) & 0XFF);

			lcd_write_addr(0x2B); 
			lcd_write_data(0x00);	                          /* y start */  
			lcd_write_data(0x00);
			lcd_write_data((LCD_DEFAULT_HEIGTH-1)>>8);	    /* y end */   
			lcd_write_data((LCD_DEFAULT_HEIGTH-1) & 0XFF);				
		  break;
	}	
	
}

/*******************************************************************
********************************************************************
* 名称： void lcd_init(void)
* 功能： 初始化ili9488驱动的lcd显示屏
* 参数： null
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_init(void)
{
	lcd_pin_init();              // lcd 的硬件pin的初始化
	lcd_fsmc_init();             // lcd 使用FSMC的初始化
	LCD_RESET = 0;               // 复位
	systick_delay_ms(100);       // 复位保持时间
	LCD_RESET = 1;	             // 解除复位
	systick_delay_ms(100);       // 解除复位保持时间
	lcd_ili9488_register_init(); // lcd各寄存器的初始化，为显示做准备
	systick_delay_ms(100);  
	lcd_clear(White);            //清屏函数
	lcd_bl_ctrl(true);           //打开背光
	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
uint32_t  lcd_read_id(void)
{
	uint16_t buf[4];
	lcd_write_addr(0x04);  
	buf[0] = lcd_read_register();        // 第一个读取数据无效
	buf[1] = lcd_read_register()&0x00ff; // 只有低8位数据有效
	buf[2] = lcd_read_register()&0x00ff; // 只有低8位数据有效
	buf[3] = lcd_read_register()&0x00ff; // 只有低8位数据有效
	return (buf[1] << 16) + (buf[2] << 8) + buf[3]; 
}


/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_clear(unsigned int color)
{
	  u32 i; 
	  lcd_ili9488_set_windows(0,0,LCD_DEFAULT_WIDTH-1,LCD_DEFAULT_HEIGTH-1);
	  /* 开始向GRAM写入数据 */
		lcd_write_addr(0x2C);
		for(i=0;i<320*480;i++)
		{
				lcd_write_data(color);
		}
}

/*******************************************************************
********************************************************************
* 名称：void lcd_set_cursor(u16 x,u16 y)
* 功能：设置显示的起始光标位置
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_set_cursor(u16 x,u16 y)
{
		lcd_write_addr(0x2A);   // 设置列坐标命令
		lcd_write_data((x&0xff00)>>8);
		lcd_write_data((x&0x00ff));
		lcd_write_data((x&0xff00)>>8);
		lcd_write_data((x&0x00ff));
		lcd_write_addr(0x2B);  // 设置page坐标命令
		lcd_write_data((y&0xff00)>>8);
		lcd_write_data((y&0x00ff));
		lcd_write_data((y&0xff00)>>8);
		lcd_write_data((y&0x00ff));
}

/*******************************************************************
********************************************************************
* 名称：void lcd_fill_color(u16 xstart,u16 ystart,u16 xend,u16 yend,unsigned int color)
* 功能：对选定的x y x1,y1区域内填充 color规定的颜色
* 参数： xstart， ystart 开始坐标 xend yend结束坐标  color：要填充的颜色
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_fill_color(u16 xstart,u16 ystart,u16 xend,u16 yend,unsigned int color)
{
	  int i,j;
	  //设置窗口
		lcd_ili9488_set_windows(xstart,ystart,xend,yend);	
	  /* 开始向GRAM写入数据 */
	  lcd_write_addr (0x2C);	
		for(j=0;j<=yend-ystart;j++)
		{
			for(i=0;i<=xend-xstart;i++)
				{
					lcd_write_data(color);			
				}
		}
}

/*******************************************************************
********************************************************************
* 名称：void lcd_drawpoint(u16 x,u16 y,u16 color)
* 功能：对所选的点填充颜色
* 参数： x，y要填充的坐标点  color：要填充的颜色
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_drawpoint(u16 x,u16 y,u16 color)
{
	  lcd_set_cursor(x,y);		//设置光标位置 
	  /* 开始向GRAM写入数据 */
	  lcd_write_addr (0x2C);
	  lcd_write_data(color);			 
}
/*******************************************************************
********************************************************************
* 名称：void lcd_draw_line(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
* 功能：划线
* 参数：x1,y1 起点坐标  x2,y2终点坐标  color：要划线的颜色
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_draw_line(u16 x1, u16 y1, u16 x2, u16 y2,u16 color)
{
	u16 t; 
	int xerr=0,yerr=0,delta_x,delta_y,distance; 
	int incx,incy,uRow,uCol; 
	delta_x=x2-x1; //计算坐标增量 
	delta_y=y2-y1; 
	uRow=x1; 
	uCol=y1; 
	if(delta_x>0)incx=1; //设置单步方向 
	else if(delta_x==0)incx=0;//垂直线 
	else {incx=-1;delta_x=-delta_x;} 
	if(delta_y>0)incy=1; 
	else if(delta_y==0)incy=0;//水平线 
	else{incy=-1;delta_y=-delta_y;} 
	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 
	else distance=delta_y; 
	for(t=0;t<=distance+1;t++ )//画线输出 
	{  
		lcd_drawpoint(uRow,uCol,color);//画点 
		xerr+=delta_x ; 
		yerr+=delta_y ; 
		if(xerr>distance) 
		{ 
			xerr-=distance; 
			uRow+=incx; 
		} 
		if(yerr>distance) 
		{ 
			yerr-=distance; 
			uCol+=incy; 
		} 
	}  
}

/*******************************************************************
********************************************************************
* 名称：void lcd_draw_rectangle(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
* 功能：画矩形
* 参数：x1 ，y1: 矩形的一个起点  x2,y2：终点，color：颜色
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_draw_rectangle(u16 x1,u16 y1,u16 x2,u16 y2,u16 color)
{
	lcd_draw_line(x1,y1,x2,y1,color);
	lcd_draw_line(x1,y1,x1,y2,color);
	lcd_draw_line(x1,y2,x2,y2,color);
	lcd_draw_line(x2,y1,x2,y2,color);
}
/*******************************************************************
********************************************************************
* 名称：void lcd_draw_circle(u16 x0,u16 y0,u8 r,u16 color)
* 功能：画圆
* 参数：x0 y0:圆心 r：半径 color 颜色
* 返回值：null
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_draw_circle(u16 x0,u16 y0,u8 r,u16 color)
{
	int a,b;
	int di;
	a=0;b=r;	  
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		lcd_drawpoint(x0+a,y0-b,color);             //5
 		lcd_drawpoint(x0+b,y0-a,color);             //0           
		lcd_drawpoint(x0+b,y0+a,color);             //4               
		lcd_drawpoint(x0+a,y0+b,color);             //6 
		lcd_drawpoint(x0-a,y0+b,color);             //1       
 		lcd_drawpoint(x0-b,y0+a,color);             
		lcd_drawpoint(x0-a,y0-b,color);             //2             
  	lcd_drawpoint(x0-b,y0-a,color);             //7     	         
		a++;
		//使用Bresenham算法画圆     
		if(di<0)di +=4*a+6;	  
		else
		{
			di+=10+4*(a-b);   
			b--;
		} 						    
	}
} 
/*******************************************************************
********************************************************************
* 名称：static void display_one_ascii(u16 x,u16 y,u8 zk_size,u32 fc,u32 bc)
* 功能：显示一个字符
* 参数：x,y 字符显示位置 zk_size大小  fc；字的颜色  bc：背景色
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static void display_one_ascii(u16 x,u16 y,u8 zk_size,u8* p_txt,u32 fc,u32 bc)
{
	u8 i,k,x_width,y_high,m=0;
	u8 n=0; //n表示不同字号下，一个单独的字符所占的字节数量
	switch(zk_size)
	{
		case 6:
				n = 12; //n表示不同字号下，一个单独的字符所占的字节数量
				x_width = 6;
				y_high = 12;
				break;
		case 8:
				n = 16; //n表示不同字号下，一个单独的字符所占的字节数量
				x_width = 8;
				y_high = 16;
				break;
		case 12:
				n = 48; //n表示不同字号下，一个单独的字符所占的字节数量
				x_width = 12;
				y_high = 24;
				break;
		case 16:
				n = 64; //n表示不同字号下，一个单独的字符所占的字节数量
				x_width = 16;
				y_high = 32;
				break;
	}
	lcd_ili9488_set_windows(x,y,x+x_width-1,y+y_high-1 );
	/* 开始向GRAM写入数据 */
	lcd_write_addr(0x2C);
	for(i = 0;i < n;i++)  // n表示不同字号下，一个单独的字符所占的字节数量
	{
		for(k=0;k<8;k++)
		{
			// 0x80>>k 意思：从最高位向右一个一个读取是否需要显示 1需要显示 0 不需要显示
			if((p_txt[i] & (0x80>>k)))  
			{
				lcd_write_data(fc);  // 向1的地方显示一个色点
			}
			else
			{
				lcd_write_data(bc);  // 向0的地方显示背景色
			}
			m++; // 记录字体宽度已经显示了的数量
			if(m % x_width == 0) // 当m的计数和字体宽度相等时，退出内循环
			{
				m=0;
				break;
			}
		
		}
	}
	
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void display_ascii_string(u16 x,u16 y,u16 zk_size,u8 text[],u32 fc,u32 bc)
{
	u8 i=0;
	u8 n_size = 0;        // 不同字符大小下，单个字符的字节数
	u8 txt_distence = 0;  // 字符间隔距离
    u32 font_addr =0;     // 字的地址
	switch(zk_size)
	{
		case 6:
			n_size = 12;
			txt_distence = 6;
			fontbuf = youyuan_6x12_L2R_H2L;
			break;
		case 8:
			n_size = 16;
			txt_distence = 8;
			fontbuf = youyuan_8x16_L2R_H2L;
			break;
		case 12:
			n_size = 48;
			txt_distence = 12;
			fontbuf = youyuan_12x24_L2R_H2L;
			break;
		case 16:
			n_size = 64;
			txt_distence = 16;
			fontbuf = youyuan_16x32_L2R_H2L;
			break;
	}
	while(text[i] > 0x00)// 还没有到字符串末尾
	{
		if((text[i] >=32 )&&(text[i] <= 126))  // 可显示字符的assic 从32 到 126，一共95个字符
		{
			font_addr = text[i] - 32 ; // 拿要显示的字符的ascii码-32，得到偏移地址
			font_addr = font_addr * n_size;
			
			display_one_ascii(x,y,zk_size,fontbuf+font_addr,fc,bc);
		}
		i++;        			 	//下一个要显示的字符
		x += txt_distence;	//下一个字要显示的起始坐标 
	}
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：图片显示函数
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_show_image(u16 x ,u16 y,const unsigned char pic[])
{
	u32 i = 8;     // 用于循环计数
	u32 len = 0;     // 用于循环计数
	u16 temp = 0;  // 用于合并颜色数据
	u16 m=0,n=0;
	m = (uint16_t )((pic[2]<<8) | pic[3]);         // 图片宽度
	n = (uint16_t )((pic[4]<<8) | pic[5]);         // 图片高度
	
	len = (uint32_t )(2*m*n+8);                    // 图片数组长度,包括图片头的8个字节数据
	
	
	lcd_ili9488_set_windows(x,y,x+m-1,y+n-1);      // 这里长宽均要-1，要不图形变形
	lcd_write_addr (0x2C);	                       /* 开始向GRAM写入数据 */
	while(i < len)
	{
		temp =(uint16_t ) ((pic[i]<<8) | pic[i+1]) ; // 合并图片数据， 2字节合成一个颜色数据
		lcd_write_data(temp);
		i += 2;                                      //取模数组+2，到下一个颜色字节
	}
}

/*******************************************************************
********************************************************************
* 名称：void lcd_display_test_result(u16 x_start,u16 y_start,u16 x_end,u16 y_end,u8 txt_size,bool result)
* 功能：根据结果显示pass  和 fail色块和字符
* 参数：x_start，y_start：色块起始坐标  x_end，y_end：色块终点坐标  txt_size：字符大小  result：测试结果
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2021-11-02
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void lcd_display_test_result(u16 x_start,u16 y_start,u16 x_end,u16 y_end,u8 txt_size,u8 result)
{
	u8 txt_hight = 0; // 记录选用的字的高度
	switch (txt_size)
	{	
		case 6:
			txt_hight = 12;
			break;
		case 8:
			txt_hight = 16;
			break;
		case 12:
			txt_hight = 24;
			break;
		case 16:
			txt_hight = 32;
			break;
	}
	if(result == PASS_FLAG) // pass
	{
		lcd_fill_color(x_start,y_start,x_end,y_end,Green);
		display_ascii_string(x_start+(x_end-x_start-4*txt_size)/2,y_start+(y_end-y_start-txt_hight)/2,txt_size,"PASS",Black,Green);
	}
	else if(result == FAIL_FLAG) //fail
	{
		lcd_fill_color(x_start,y_start,x_end,y_end,Red);
		display_ascii_string(x_start+(x_end-x_start-4*txt_size)/2,y_start+(y_end-y_start-txt_hight)/2,txt_size,"FAIL",Black,Red);
	}
	else if(result == STDBY_FLAG)// stdby
	{
		lcd_fill_color(x_start,y_start,x_end,y_end,DarkGray);
		display_ascii_string(x_start+(x_end-x_start-5*txt_size)/2,y_start+(y_end-y_start-txt_hight)/2,txt_size,"STDBY",White,DarkGray);
	
	}
	else //testing 
	{
		lcd_fill_color(x_start,y_start,x_end,y_end,Yellow);
		display_ascii_string(x_start+(x_end-x_start-7*txt_size)/2,y_start+(y_end-y_start-txt_hight)/2,txt_size,"TESTING",Blue,Yellow);
	}

}
