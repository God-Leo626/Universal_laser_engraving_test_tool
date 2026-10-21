#ifndef __ADC_H__
#define __ADC_H__
/*====================================================================*/
#define ADC_CHANNEL_NUM 4
extern  volatile  uint16_t adc_read_data[ADC_CHANNEL_NUM] ;	// 用于保存ADC1读取到的电压数据

/*====================================================================*/

void adc1_init(void);

#endif


