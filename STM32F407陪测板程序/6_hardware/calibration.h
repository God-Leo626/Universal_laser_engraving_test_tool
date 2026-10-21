#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__
#include "stm32f4xx.h" 
#include "io_bit.h"
#include "stdbool.h"
/*====================================================================*/
#define CTRL_PC6  PCout(6)       // 10M
#define CTRL_PC7  PCout(7)       // 1M
#define CTRL_PC8  PCout(8)       // 100K
#define CTRL_PC9  PCout(9)       // 10K

#define CTRL_PA8   PAout(8)       // 1K
#define CTRL_PA11  PAout(11)      // 100
#define CTRL_PA12  PAout(12)      // 20


#define R10M    0    //  10mµç×è
#define R1M     1   //  1mµç×è
#define R100K   2   // 100k
#define R10K    3   // 10k
#define R1K     4   // 1k
#define R100    5   // 100
#define R20     6   // 20

/*====================================================================*/

void current_calibration_init(void);
void calibration_ctrl(u8 resistor_select);


#endif
