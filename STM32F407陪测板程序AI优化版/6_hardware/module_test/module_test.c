#include "module_test.h"
#include "stm32f10x.h"
#include  "stdbool.h"

#include "usart_dma_all.h"
// PD3   RESET
// PD2   BOOT  
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
* // PD3   RESET
  // PD2   BOOT  
* 
*********************************************************************
*********************************************************************/
void  module_ctrl_reset_boot_init(void)
{
	// PD2  BOOT
	GPIO_InitTypeDef GPIO_InitTypeDef_init;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);  // 使能gpio D 口时钟
	GPIO_InitTypeDef_init.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitTypeDef_init.GPIO_Pin =  GPIO_Pin_2;
	GPIO_InitTypeDef_init.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD,&GPIO_InitTypeDef_init);
	
	// PD3 RESET
	GPIO_InitTypeDef_init.GPIO_Pin =  GPIO_Pin_3;
	GPIO_Init(GPIOD,&GPIO_InitTypeDef_init);
	
}
//**************************************************************************************
//    PB3  PB4 PB5 PB6 PB7 PB8  PB9  PE0  PE1 PE6  PE4 PC13 PE5  PE3 PC2 PC3 PA0 PA1  PC0  PC4  PC5  PB0 PA8  PC9
//bit 0    1   2   3   4   5    6    7    8   9   10   11   12   13  14  15   16  17   18   19   20   21  22   23
u32 IO_TEST_RESULT_H = 0 ; // 每个bit记录一个io口的测试 高电平 的结果
u32 IO_TEST_RESULT_L = 0 ; // 每个bit记录一个io口的测试 低电平 的结果
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
*   J12 座子 :PB3  PB4 PB5 PB6 PB7 PB8  PB9  PE0  PE1
    J13 座子 : PE6  PE4 PC13 PE5 PE3 PC2 PC3 PA0 PA1  PC0  PC4  PC5  PB0 PA8  PC9
*********************************************************************
*********************************************************************/
static void module_io_test_pin_init_L(void) //测试低电平
{
	GPIO_InitTypeDef GPIO_InitTypeDef_init;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE,ENABLE);  // 使能gpio A E 口时钟
	// PB3初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitTypeDef_init.GPIO_Speed =  GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB4 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB5 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB6 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB7 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB7 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB8 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB9 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PB7 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	//======================================
	// PE6 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PE4 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PC13初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PE5 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PE3初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PC2 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PC3初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PA0 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_init);
	// PA1 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_init);
	// PC0初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PC4 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PC5 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PB0 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PA8 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_init);
	// PC9 初始化为上拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
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
static void module_io_test_pin_init_H(void) //测试高电平
{
	GPIO_InitTypeDef GPIO_InitTypeDef_init;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE,ENABLE);  // 使能gpio A E 口时钟
	// PB3初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitTypeDef_init.GPIO_Speed =  GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB4 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB5 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB6 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB7 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB7 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB8 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PB9 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PB7 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	
	//======================================
	// PE6 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PE4 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PC13初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PE5 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PE3初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOE,&GPIO_InitTypeDef_init);
	// PC2 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PC3初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PA0 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_init);
	// PA1 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_init);
	// PC0初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PC4 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PC5 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
	// PB0 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB,&GPIO_InitTypeDef_init);
	// PA8 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA,&GPIO_InitTypeDef_init);
	// PC9 初始化为下拉
	GPIO_InitTypeDef_init.GPIO_Pin = GPIO_Pin_9;
	GPIO_Init(GPIOC,&GPIO_InitTypeDef_init);
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2022-11-02
* 备注：
* 
* TEST1:PB0 PB1 PC6 
*********************************************************************
*********************************************************************/
void module_io_test_H(u8 io_test_number)
{
	module_io_test_pin_init_H();
	switch(io_test_number)
	{
		case 0:
				IO_TEST_RESULT_H |=  0xffffffff;  // 全部置1，表示io H 测试 全部通过 （H）
				break;
		case 1:
				if(PB3 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				break;
		case 2:
				if(PB3 == 1 && PB4 == 0)
				{
					IO_TEST_RESULT_H |= 0x00000001;    // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				break;
		case 3:
				if(PB3 == 1 && PB4 == 0 && PB5 == 0 )
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				break;
		case 4:
				if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 )
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				break;
		case 5:
				if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 )
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				break;
		case 6:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 )
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1 && PB8 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				break;
		case 7:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 )
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0 && PB9 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1 && PB8 == 0 && PB9 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1 && PB9 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000040;   // pb9 H 测试ok
				}
				break;
		case 8:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1 && PB8 == 0 && PB9 == 0 && PE0 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1 && PB9 == 0 && PE0 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 1 && PE0 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000080;   // pE0 H 测试ok
				}
				break;
		case 9:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1&& PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0&& PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1 && PB9 == 0 && PE0 == 0 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 1 && PE0 == 0 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 01 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 1 && PE1 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000100;   // pE1 H 测试ok
				}
				break;
		case 10:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 1 && PE0 == 0 && PE1 == 0 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 1 && PE1 == 0 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 1 && PE6== 0)
				{
					IO_TEST_RESULT_H |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6== 1)
				{
					IO_TEST_RESULT_H |=  0x00000200;   // pE6 H 测试ok
				}
				break;
		case 11:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1&& PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 1 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 1 && PE1 == 0 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 1 && PE6 == 0 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 1 && PE4 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000200;   // pE6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000400;   // pE6 H 测试ok
				}
				break;
		case 12:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 1 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 1 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 1 && PE6 == 0 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 1 && PE4 == 0 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000200;   // pE6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 1 && PC13 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000400;   // pE6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 1)
				{
					IO_TEST_RESULT_H |=  0x00000800;   // pE6 H 测试ok
				}
		case 13:
			    if(PB3 == 1 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 1 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 1 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 1 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 1 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 1 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 1 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 1 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 1 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 1 && PE4 == 0 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000200;   // pE6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 1 && PC13 == 0 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000400;   // pE6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 1 && PE5 == 0)
				{
					IO_TEST_RESULT_H |=  0x00000800;   // pE6 H 测试ok
				}
				else if(PB3 == 0 && PB4 == 0 && PB5 == 0 && PB6 == 0 && PB7 == 0 && PB8 == 0 && PB9 == 0 && PE0 == 0 && PE1 == 0 && PE6 == 0 && PE4 == 0 && PC13 == 0 && PE5 == 1)
				{
					IO_TEST_RESULT_H |=  0x00001000;   // pE6 H 测试ok
				}
				break;
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
* 日期：2022-11-02
* 备注：
* 
* TEST1:PB0 PB1 PC6 
*********************************************************************
*********************************************************************/
void module_io_test_L(u8 io_test_number)
{
	module_io_test_pin_init_L();
	switch(io_test_number)
	{
		case 0:
				IO_TEST_RESULT_L |=  0xffffffff;  // 全部置1，表示io H 测试 全部通过 （H）
				break;
		case 1:
				if(PB3 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				break;
		case 2:
				if(PB3 == 0 && PB4 == 1)
				{
					IO_TEST_RESULT_L |= 0x00000001;    // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				break;
		case 3:
				if(PB3 == 0 && PB4 == 1 && PB5 == 1 )
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				break;
		case 4:
				if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1 )
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				break;
		case 5:
				if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1 )
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1 && PB7 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				break;
		case 6:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 )
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1 && PB8 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1 && PB8 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0 && PB8 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				break;
		case 7:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 )
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1 && PB8 == 1 && PB9 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0 && PB8 == 1 && PB9 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 0 && PB9 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000040;   // pb9 H 测试ok
				}
				break;
		case 8:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0 && PB8 == 1 && PB9 == 1 && PE0 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 ==1 && PB6 == 1 && PB7 == 1 && PB8 == 0 && PB9 == 1 && PE0 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 0 && PE0 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1&& PB5 == 1 && PB6 == 1 && PB7 ==1 && PB8 == 1 && PB9 == 1 && PE0 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000080;   // pE0 H 测试ok
				}
				break;
		case 9:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1&& PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0&& PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 0 && PB9 == 1 && PE0 == 1 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 0 && PE0 == 1 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 0 && PE1 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000100;   // pE1 H 测试ok
				}
				break;
		case 10:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 0 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 0 && PE0 == 1 && PE1 == 1 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 0 && PE1 == 1 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 0 && PE6== 1)
				{
					IO_TEST_RESULT_L |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6== 0)
				{
					IO_TEST_RESULT_L |=  0x00000200;   // pE6 H 测试ok
				}
				break;
		case 11:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1&& PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1&& PE6 == 1&& PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 ==1 && PB7 == 1 && PB8 == 1 && PB9 == 1&& PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1&& PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0&& PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 0 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1&& PB7 == 1 && PB8 == 1 && PB9 == 0 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1&& PB5 == 1&& PB6 == 1 && PB7 == 1 && PB8 ==1&& PB9 == 1 && PE0 == 0 && PE1 == 1 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 0 && PE6 == 1 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 1&& PB4 == 1 && PB5 == 1&& PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1&& PE1 == 1&& PE6 == 0 && PE4 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000200;   // pE6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000400;   // pE6 H 测试ok
				}
				break;
		case 12:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 01&& PB6 == 1&& PB7 == 1 && PB8 == 1 && PB9 == 1&& PE0 == 1 && PE1 == 1&& PE6 == 1 && PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1&& PE6 == 1 && PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1&& PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1&& PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 0 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1&& PB6 == 1 && PB7 == 0 && PB8 == 1 && PB9 == 1&& PE0 == 1&& PE1 == 1 && PE6 == 1&& PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1&& PB6 == 1 && PB7 == 1 && PB8 == 0 && PB9 == 1 && PE0 == 1 && PE1 == 1&& PE6 == 1 && PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1&& PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 0 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 ==1)
				{
					IO_TEST_RESULT_L |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 0 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 0 && PE6 == 1 && PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1&& PE6 == 0 && PE4 == 1 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000200;   // pE6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1&& PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1&& PE6 == 1&& PE4 == 0 && PC13 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000400;   // pE6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 0)
				{
					IO_TEST_RESULT_L |=  0x00000800;   // pE6 H 测试ok
				}
		case 13:
			    if(PB3 == 0 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000001 ; // pb3 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 0 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000002;   // pb4 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 0 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1&& PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000004;   // pb5 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1&& PB6 == 0 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000008;   // pb6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 0 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000010;   // pb7 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 0 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000020;   // pb8 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 0 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000040;   // pb9 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 0 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000080;   // pE0 H 测试ok
				}
				else if(PB3 == 1&& PB4 == 1 && PB5 == 1 && PB6 == 1&& PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 0 && PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000100;   // pE1 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1&& PB6 == 1 && PB7 == 1&& PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 0 && PE4 == 1 && PC13 == 1 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000200;   // pE6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1&& PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 0 && PC13 == 1 && PE5 ==1)
				{
					IO_TEST_RESULT_L |=  0x00000400;   // pE6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1&& PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1 && PE6 == 1 && PE4 == 1 && PC13 == 0 && PE5 == 1)
				{
					IO_TEST_RESULT_L |=  0x00000800;   // pE6 H 测试ok
				}
				else if(PB3 == 1 && PB4 == 1 && PB5 == 1 && PB6 == 1 && PB7 == 1 && PB8 == 1 && PB9 == 1 && PE0 == 1 && PE1 == 1&& PE6 == 1 && PE4 == 1 && PC13 == 1 && PE5 == 0)
				{
					IO_TEST_RESULT_L |=  0x00001000;   // pE6 H 测试ok
				}
				break;
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
* 日期：2022-09-05
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
bool get_module_test_result(u8 io_test_number)
{
	switch(io_test_number)
	{
		case 0:
				return true ;                                                            // 不测试 io
		case 1:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x00000001) == 0x00000001 )   // 测试1个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 2:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x00000003) == 0x00000003)   // 测试2个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 3:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x00000007) == 0x00000007)  // 测试3个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 4:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x0000000f) == 0x0000000f)   // 测试4个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 5:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x0000001f) == 0x0000001f)  // 测试5个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 6:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x0000003f) == 0x0000003f) // 测试6个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 7:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x0000007f) == 0x0000007f)  // 测试7个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 8:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x000000ff) == 0x000000ff)  // 测试8个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 9:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x000001ff) == 0x000001ff)  // 测试9个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 10:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x000003ff) == 0x000003ff)  // 测试10个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 11:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x000007ff) == 0x000007ff)  // 测试11个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 12:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x00000fff) == 0x00000fff)  // 测试12个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		case 13:
				if(((IO_TEST_RESULT_H & IO_TEST_RESULT_L)& 0x00001fff) == 0x00001fff)  // 测试13个 io
				{
					return true ;
				}
				else
				{
					return false;
				}
		default:
			usart1_printf("io_test_number select error [0 ---13 ]\r\n");
			return  false;
	
			
	}
}




