#ifndef   __MODULE_TEST_H__
#define  __MODULE_TEST_H__

#include "io_bit.h"
#include "stm32f4xx.h"
#include "stdbool.h"
//**************************************************************************************
// PD3   RESET
// PD2   BOOT  
#define MODULE_RESET   PDout(3)
#define MODULE_BOOT    PDout(2)

//**************************************************************************************
extern u32 IO_TEST_RESULT_H  ; // 每个bit记录一个io口的测试 高电平 的结果
extern u32 IO_TEST_RESULT_L  ; // 每个bit记录一个io口的测试 低电平 的结果

//**************************************************************************************
//    J12 座子 :PB3  PB4 PB5 PB6 PB7 PB8  PB9  PE0  PE1
//    J13 座子 : PE6  PE4 PC13 PE5 PE3 PC2 PC3 PA0 PA1  PC0  PC4  PC5  PB0 PA8  PC9

#define PB3   PBin(3) 
#define PB4   PBin(4) 
#define PB5   PBin(5) 
#define PB6   PBin(6) 
#define PB7   PBin(7) 
#define PB8   PBin(8) 
#define PB9   PBin(9) 

#define PE0   PEin(0) 
#define PE1   PEin(1) 

#define PE6     PEin(6) 
#define PE4     PEin(4) 
//#define PC13    PCin(13) 
#define PE5     PEin(5)   
#define PE3     PEin(3) 
#define PC2     PCin(2) 
#define PC3     PCin(3) 
#define PA0     PAin(0) 
#define PA1     P1in(1) 
#define PC0     PCin(0) 
#define PC4     PCin(4) 
#define PC5     PCin(5) 
#define PB0     PBin(0) 
#define PA8     PAin(8) 
#define PC9     PCin(9) 
//**************************************************************************************
void module_ctrl_reset_boot_init(void);
void module_io_test_H(u8 io_test_number);
void module_io_test_L(u8 io_test_number);
bool get_module_test_result(u8 io_test_number);
void moudule_reset_ctrl(u8 reset_time);
#endif


