#include "adc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_rcc.h"
#include "string.h"
volatile  uint16_t adc_read_data[ADC_CHANNEL_NUM] = {0};	// 用于保存ADC1读取到的电压数据
/*====================================================================*/


/*******************************************************************
********************************************************************
* 名称：void adc1_init(void)
* 功能：初始化adc1，并使能dma功能（PA0 PA1 PA2 PC3）四个adc通道扫描模式读取adc值
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
PA0 : 通道0
PA1 ：通道1
PA2 ：通道2
PC3 ：通道13
  (ADC1 PIN)
*********************************************************************
*********************************************************************/
void adc1_init(void)
{
    DMA_InitTypeDef        DMA_InitStructure;
	ADC_CommonInitTypeDef  ADC_CommonInitStructure;
	ADC_InitTypeDef        ADC_InitStructure;	
    GPIO_InitTypeDef       GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOC,ENABLE); //使能GPIOA  GPIOC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);                      //使能ADC1时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);                      //使能DMA2对应的时钟 (adc1 2 3 的dma请求在DMA2中)
   
    // 初始化ADC1 的gpio口 PA0 PA1 PA2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;//不带上下拉
    GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化

    // 初始化ADC1 的gpio口 PC3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化

    DMA_DeInit(DMA2_Stream0);                                               // ADC1 的映射时通道0，数据流0  DMA 流的所有寄存器都会被重置为默认值
    DMA_InitStructure.DMA_Channel = DMA_Channel_0 ;                         // ADC1 的映射时通道0，数据流0
    // DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & ADC1->DR ;      // adc1 的外设地址 
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) ((uint32_t)0x4001204C) ;      // adc1 的外设地址 
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)adc_read_data ;       // ADC 转换数据接收地址
    DMA_InitStructure.DMA_DIR =  DMA_DIR_PeripheralToMemory              ;  // DMA传输方向： 外设--->内存
    DMA_InitStructure.DMA_BufferSize = ADC_CHANNEL_NUM; //数据传输字节数量
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;             //外设非增量模式
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                      //存储器增量模式
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //外设数据长度:16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	         //存储器数据长度:16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;                              //循环传输模式 ???
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;                          //高优先级 
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;                       //禁用fifo
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;            //将 FIFO 阈值设置为 “满” 状态,意味着 DMA 在 FIFO 达到满状态时才会进行数据传输操作。
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;                  //存储器突发单次传输
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;          //外设突发单次传输
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);                                  //初始化DMA流
    DMA_Cmd(DMA2_Stream0, ENABLE);                                               //开启DMA传输

    // RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,ENABLE);	    //ADC1复位
	// RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1,DISABLE);	//复位结束	
    ADC_DeInit();  // 复位ADC
    ADC_CommonInitStructure.ADC_Mode =  ADC_Mode_Independent;  //独立模式
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//预分频4分频。ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz 
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; //DMA使能 {****}
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//两个采样阶段之间的延迟5个时钟
    ADC_CommonInit(&ADC_CommonInitStructure);//初始化

    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;//12位模式
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;          //扫描模式开启，用于多通道采集
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;    //连续转换
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                     //右对齐
    // ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;      // adc 外部触发选择  ？？
	ADC_InitStructure.ADC_NbrOfConversion = ADC_CHANNEL_NUM;                   //转换4个通道
    ADC_Init(ADC1, &ADC_InitStructure);                                        //ADC初始化	

     //配置ADC的转换通道顺序和采样周期
    ADC_RegularChannelConfig(ADC1,  ADC_Channel_0, 1, ADC_SampleTime_480Cycles);   // PA0 通道0   采样时间   42k采样
    ADC_RegularChannelConfig(ADC1,  ADC_Channel_1, 2, ADC_SampleTime_480Cycles);   // PA1 通道1  采样时间
    ADC_RegularChannelConfig(ADC1,  ADC_Channel_2, 3, ADC_SampleTime_480Cycles);   // PA2 通道2  采样时间
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_480Cycles);   // PC3 通道13  采样时间

    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE); //ADC 的最后一次转换完成后，使能 DMA 请求
    ADC_DMACmd(ADC1, ENABLE);   // 使能ADC的DMA功能
    ADC_Cmd(ADC1, ENABLE);     //开启ADC转换器
    ADC_SoftwareStartConv(ADC1);  //开始ADC1转换	
    //while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC));//等待转换结束
}


