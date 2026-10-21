#ifndef __ADS1262_H__
#define __ADS1262_H__
#include "stm32f4xx.h"
#include "io_bit.h"
#include "stdbool.h"
//===========================================================================

extern u8 threshold_ch1_global_flag  ;          // ����ͳ��ch1 �Ĵﵽ��ֵ����ֵ�Ĵ���
extern u8 threshold_ch2_global_flag  ;          // ����ͳ��ch2 �Ĵﵽ��ֵ����ֵ�Ĵ���
extern u8 threshold_ch3_global_flag  ;          // ����ͳ��ch3 �Ĵﵽ��ֵ����ֵ�Ĵ���
extern u8 threshold_ch4_global_flag  ;          // ����ͳ��ch4 �Ĵﵽ��ֵ����ֵ�Ĵ���

extern bool ch1_test_circle_global_flag  ;   // ch1�����н��ۺ󣬲��ڲ���ѭ���ı�־ ��true: ����ѭ������ false���������β��ԣ�
extern bool ch2_test_circle_global_flag  ;   // ch2�����н��ۺ󣬲��ڲ���ѭ���ı�־  ��true: ����ѭ������ false���������β��ԣ�
extern bool ch3_test_circle_global_flag  ;   // ch3�����н��ۺ󣬲��ڲ���ѭ���ı�־ ��true: ����ѭ������ false���������β��ԣ�
extern bool ch4_test_circle_global_flag  ;   // ch4�����н��ۺ󣬲��ڲ���ѭ���ı�־  ��true: ����ѭ������ false���������β��ԣ�

extern bool continuous_test_mode_global_flag  ; //  ��������ģʽ��־  ��Ĭ�ϲ�������ģʽ�� false �� ���������ԣ� true ���������ԣ�
extern u8   channel_4_test_over_global_flag  ;   // �˱�־ֻҪ���ڼ�¼4��ͨ���Ĳ����Ƿ���ϣ����۳ɹ���񣩣�4��ͨ��ȫ��������Ϲ����?���˱�־�رյ�Դ

extern double function_l_global_threshold_value ;  // ���ڱ���atof������ת�������� l ��ֵ
extern double function_h_global_threshold_value ;  // ���ڱ���atof������ת�������� h ��ֵ

// extern bool ch1_auto_20ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��
// extern bool ch2_auto_20ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��
// extern bool ch3_auto_20ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��
// extern bool ch4_auto_20ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��

// extern bool ch1_auto_200ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��
// extern bool ch2_auto_200ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��
// extern bool ch3_auto_200ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��
// extern bool ch4_auto_200ma_range_select_global_flag  ;  //  CH1�Զ�����ѡ���ʶ��û�г���? 20 ma  ��200ma ����ʱΪ�棬��ʼ��Ϊ��

extern bool ch1_auto_range_select_global_flag  ;  // ch1 �Զ�����ѡ���ʶ��û�г������?�棬��ʼ��Ϊ��
extern bool ch2_auto_range_select_global_flag  ;  // ch2 �Զ�����ѡ���ʶ��û�г������?�棬��ʼ��Ϊ��
extern bool ch3_auto_range_select_global_flag  ;  // ch3 �Զ�����ѡ���ʶ��û�г������?�棬��ʼ��Ϊ��
extern bool ch4_auto_range_select_global_flag  ;  // ch4 �Զ�����ѡ���ʶ��û�г������?�棬��ʼ��Ϊ��

extern u8 ch1_test_range_value ; // ͨ��1��������ѡ��ʱ��ȷ������������
extern u8 ch2_test_range_value  ; // ͨ��2��������ѡ��ʱ��ȷ������������
extern u8 ch3_test_range_value  ; // ͨ��3��������ѡ��ʱ��ȷ������������
extern u8 ch4_test_range_value  ; // ͨ��4��������ѡ��ʱ��ȷ������������

//*****************************************************************************************************************
#define MODE2_DATA_RATE_2_5SPS     0X00
#define MODE2_DATA_RATE_5SPS       0X01
#define MODE2_DATA_RATE_10SPS      0X02
#define MODE2_DATA_RATE_16_SPS     0X03
#define MODE2_DATA_RATE_20SPS      0X04
#define MODE2_DATA_RATE_50SPS      0X05
#define MODE2_DATA_RATE_60SPS      0X06
#define MODE2_DATA_RATE_100SPS     0X07
#define MODE2_DATA_RATE_400SPS     0X08
#define MODE2_DATA_RATE_1200SPS    0X09
#define MODE2_DATA_RATE_2400SPS    0X0A
#define MODE2_DATA_RATE_4800SPS    0X0B
#define MODE2_DATA_RATE_7200SPS    0X0C
#define MODE2_DATA_RATE_14400SPS   0X0D
#define MODE2_DATA_RATE_19200SPS   0X0E
#define MODE2_DATA_RATE_38400SPS   0X0F

//===========================================================================

#define AVERAGE_DATA_LENG     15        //  adcҪ��ȡ���ܵĵ�ѹ���ݣ�ȥ�����? ����Сֵ������ȡƽ��ֵ
#define INT_REF_VOLTAGE       2.49771    // �ڲ��ο���ѹֵ
#define EXT_REF_VOLTAGE       2.50000    // �ⲿ�ο���ѹֵ external


#define SENSE_RESISTOR_CH1        0.1      // ��������   100mR
#define SENSE_RESISTOR_CH2        0.1      // ��������   100mR
#define SENSE_RESISTOR_CH3        0.1      // ��������   100mR
#define SENSE_RESISTOR_CH4        0.1      // ��������   100mR





//===========================================================================
#define ADS1262_RESET        PEout(0)   // PE0   RESET
#define ADS1262_CS           PBout(12)  // PB12  CS  CS-��ȷ�����нӿڣ�������RST��START�ź�
#define ADS1262_READY        PEin(1)    // PE1   ready
#define ADS1262_START        PBout(9)   // PB9   START

//==============================INPUT CHANNEL =============================================
#define INPUT_CHANNEL_1             0X89   // ͨ��ѡ���ֵ���Ĵ�������ֵ��?
#define INPUT_CHANNEL_2             0X23   // ͨ��ѡ���ֵ���Ĵ�������ֵ��?
#define INPUT_CHANNEL_3             0X45   // ͨ��ѡ���ֵ���Ĵ�������ֵ��?
#define INPUT_CHANNEL_4             0X67   // ͨ��ѡ���ֵ���Ĵ�������ֵ��?
//==============================cmd=============================================
#define CMD_RESET        0X06                             //��λadc����
#define CMD_START_ADC1   0X08                             //��ʼADC1 ת������
#define CMD_STOP_ADC1    0X0A                             //ֹͣADC1 ת������
#define CMD_READ_DATA    0X12                             //��adc1��������
#define CMD_SYOCAL1      0X16                             //adc1ϵͳƫ��У׼
#define CMD_SYGCAL1      0X17                             //adc1ϵͳ����У׼
#define CMD_SFOCAL1      0X19                             //adc1��ƫ��У׼ ����      
#define CMD_READ_REG     0X20                             //���Ĵ�������
#define CMD_WRITE_REG    0X40                             //д���Ĵ�������

//=========================�Ĵ�����ַ==================================================
//OFCAL[2:0]��FSCAL[2:0]�Ĵ����顣OFCAL[2:0]��FSCAL[2:0]����ADS1262ƫ�ƺ�������У����
//ADS1262�ɸ���OFCAL[2:0]��FSCAL[2:0]�е�У��ϵ�������ݽ����Զ�У��

//INTERFACE�Ĵ������û�ͨ������INTERFACE�Ĵ�����INTERFACE.2��INTERFACE.0λ��������֡���ȣ�
//���Ƿ����STATUS״̬�ֽں�CRC�ֽڣ�����֡�Ϊ6�ֽڣ�����?4�ֽڣ�STATUS�ֽڰ�����ǰ�����?
//���Ƿ�������ȡ��PGA����Ƿ������ADC��λ״̬����Ҫ��Ϣ��

//���˲�ģʽѡ��sinc3��A/D����������Ϊ1200 S/s�ҿ���ն������0ģʽʱ������������ȶ�?
#define ID									(0x00)			//оƬid��
#define POWER 								(0x01)			// �ڲ��ο���ѹ����λ״̬��ʾ
#define INTERFACE							(0x02)			//������status�ֽڼ�crc�ֽڵ�ѡ��
#define MODE0								(0x03)			//����ģʽ��������ģʽ������ת����ʱѡ��
#define MODE1								(0x04)			//�����˲�ģʽѡ�񼰴�����ƫ�õ���Դ����
#define MODE2								(0x05)			//PGA���� ϵ����adc��������ѡ��
#define INPMUX								(0x06)			//ģ������Ķ�·ѡ��?
#define OFCAL0								(0x07)			//У���󣬴洢adc��ʼƫ��ֵ
#define OFCAL1								(0x08)			
#define OFCAL2								(0x09)			
#define FSCAL0								(0x0A)			// У���󣬴洢����������ϵ��ֵ
#define FSCAL1								(0x0B)            
#define FSCAL2								(0x0C)
#define IDACMUX								(0x0D)          //˫��������Դģ�������·ѡ��?
#define IDACMAG								(0x0E)          //idac1  idac2�ļ���������Сѡ��
#define REFMUX								(0x0F)          //�ⲿ�ο���ѹ�������·ѡ����?������
#define TDACP								(0x10)          //TDACP�������Ӽ����Ե�ѹ��С��ѡ��
#define TDACN								(0x11)          //TDACN�������Ӽ����Ե�ѹ��С��ѡ��
#define GPIOCON								(0x12)          // gpio��������������
#define GPIODIR								(0x13)          // gpio��������������ѡ��
#define GPIODAT								(0x14)          //gpio�ڵ�ƽ״̬��ʾ
	
//===========================================================================	
void get_ads1262_id(void);
void ads1262_hd_reset(void);
void ads1262_soft_reset(void);

void ads1262_start_hd_adc1(void);
void ads1262_stop_hd_adc1(void);
void ads1262_stop_adc1(void);
u32 ads1262_read_adc1_conversion_data(void);
void ads1262_init(void);
void ads1262_read_register(u8 start_address,u8 num_reg,u8* p_data);	 
void ads1262_read_direct_adc1_conversion_data(u8 num_bytes,u8 data_bytes_start_num);
void wait_for_ads1262_adc1_conversion_ready(void);
void  ads1262_adc1_cmd_read_data(u8 adc_channel,u8 read_data_number, double Threshold_l_value,double Threshold_h_value,u8 Threshold_test_number);
void ads1262_calibration_init(void);
void ads1262_SYOCAL1_calibration(u8 channel_select);
void ads1262_read_data_auto_range_select(u8 adc_channel);
void ads1262_auto_range_select(u8 adc_channel) ;
#endif


