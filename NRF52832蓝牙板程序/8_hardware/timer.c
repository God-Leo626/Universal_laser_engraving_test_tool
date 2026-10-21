

#include "sdk_config.h"
#include "timer.h"
#include "stdint.h"
#include "stdbool.h"

#include "led.h"
#include "nrf_drv_timer.h"
//#include "nrf_52832_uarte.h"
#include "my_printf.h"
#include <nrfx_timer.h> 
//*****************************************************************************************************************
//����Timer0����������ʵ������������ʵ����ID��ӦTimer��ID����NRF_DRV_TIMER_INSTANCE(0)��ӦTimer0
const nrfx_timer_t TIMER_INSTANCE = NRFX_TIMER_INSTANCE(TIMER1);

/*******************************************************************
********************************************************************
* ���ƣ� void timer_event_handler(nrf_timer_event_t event_type, void* p_context)
* ���ܣ� Timer�¼��ص�����
* ������
* ����ֵ��
* �汾��v1
* ���ߣ�matt
* ���ڣ�2024-02-16
* ������
* ��ע��
* 
* 
*********************************************************************
*********************************************************************/
//void timer_event_handler(nrf_timer_event_t event_type, void* p_context)
//{
//	// ��ʱ���Ƚ�ƥ���жϣ�1ms ��ʱ��
//	if(event_type == NRF_TIMER_EVENT_COMPARE0)
//	{
//		if(nrf_uarte_event_check(m_uarte.p_reg, NRF_UARTE_EVENT_RXDRDY) == 0)  // �ж� EVENTS_RXDRDY ״̬,0 :��ʾû���յ��ֽ�����
//		{
//			print("timer1\r\n") ;
////			nrf_uarte_task_trigger(m_uarte.p_reg, NRF_UARTE_TASK_STOPRX);      // ���� STOPRX �� Ӳ��ֹͣ DMA �� �Զ���λ ENDRX �� ���� NRFX_UARTE_EVT_RX_DONE �ص���
//		}
//		else
//		{
//			print("timer2\r\n") ;
////			nrf_uarte_event_clear(m_uarte.p_reg, NRF_UARTE_EVENT_RXDRDY);      // �����־���������    
//		}
//		
//	}
//	
//}

/*******************************************************************
********************************************************************
* ���ƣ� void timer_init(uint32_t timer_num_ms)
* ���ܣ� ��ʱ����ʼ����������ȷ����ʱ��ʱ�䣬��λms
* ������ timeout_us �� Ҫ��ʱ��ʱ�䣬��λus
irq_priority :���ȼ�
handler ���ص�����
* ����ֵ��
* �汾��v1
* ���ߣ�matt
* ���ڣ�2026-06-06
* ������
* ��ע��
* 
* ��ģ�鲻����������ʱ���������� nrfx_timer_enable����
 *       ������ PPI �� RXSTARTED ʱͨ�� TASK_START ������
 *       ���� RX ����ǰ������ٳ�ʱ
*********************************************************************
*********************************************************************/
void timer1_init(uint32_t timeout_us,uint8_t  irq_priority,nrfx_timer_event_handler_t  handler)
{
	uint32_t err_code = NRF_SUCCESS;
	uint32_t time_ticks	; //��ʱʱ��
	

	
    nrfx_timer_uninit(&TIMER_INSTANCE);  // ��λ��ʱ��
	//���嶨ʱ�����ýṹ�壬��ʹ��Ĭ�����ò�����ʼ���ṹ��
	// ��ʱ������س�ʼ�������������������
	nrfx_timer_config_t timer_cfg ;
	timer_cfg.frequency          = NRF_TIMER_FREQ_1MHz;   /* 1 MHz �� 1 tick = 1 ��s�������㹻 */
	timer_cfg.mode               = NRF_TIMER_MODE_TIMER;  /* ��׼��ʱ��ģʽ�����ϼ����� */
	timer_cfg.bit_width          = NRF_TIMER_BIT_WIDTH_32;/* 32 λ �� ���ʱԼ 71 ���� */
	timer_cfg.interrupt_priority = irq_priority;          /* NVIC ���ȼ� */
	timer_cfg.p_context          = NULL;                  /* ������ָ�룬��ʹ�� */
	//��ʼ����ʱ������ʼ��ʱ��ע��timer_event_handler�¼��ص�����
	err_code = nrfx_timer_init(
								&TIMER_INSTANCE,                      // ��ʱ��������ʵ��
								&timer_cfg,                           // ��ʱ�����ýṹ�� ��ʹ��Ĭ�����ò���
								handler);                             // ��ʱ���ص�����
	APP_ERROR_CHECK(err_code);
	
	time_ticks = nrfx_timer_us_to_ticks(&TIMER_INSTANCE, timeout_us);    //��ʱʱ��(��λus)ת��Ϊticks
	//���ö�ʱ������/�Ƚ�ͨ������ͨ���ıȽ�ֵ��ʹ��ͨ���ıȽ��ж�
//	nrfx_timer_extended_compare(
//								&TIMER_INSTANCE,                      // ��ʱ��������ʵ��
//								NRF_TIMER_CC_CHANNEL0,                // cc ͨ��
//								time_ticks,                           // �Ƚ�ֵ ����ʱʱ�任��Ϊ tick��
//								NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,  // ��ݷ�ʽ ���Ƚ�ƥ��ʱ�����ʱ�� ֹͣ��ʱ��   /  ��������������¿�ʼ������
//								true);                                // ������رձȽ�ͨ�����ж�


    nrfx_timer_compare(&TIMER_INSTANCE,           // ��ʱ��������ʵ��
                       NRF_TIMER_CC_CHANNEL0,     // cc ͨ��
					   time_ticks,                // �Ƚ�ֵ ����ʱʱ�任��Ϊ tick��
					   true);                     // ������رձȽ�ͨ�����ж�
	timer1_stop();
} 

/*******************************************************************
********************************************************************
* ���ƣ�void timer_stop(void)
* ���ܣ� �رն�ʱ��
* ������
* ����ֵ��
* �汾��v1
* ���ߣ�matt
* ���ڣ�2026-06-06
* ������
* ��ע��
* 
* 
*********************************************************************
*********************************************************************/
void timer1_stop(void)
{
	nrfx_timer_disable(&TIMER_INSTANCE);
}
/*******************************************************************
********************************************************************
* ���ƣ� void timer_clear(void)
* ���ܣ� ��ռ�����
* ������
* ����ֵ��
* �汾��v1
* ���ߣ�matt
* ���ڣ�2026-06-06
* ������
* ��ע��
* 
* 
*********************************************************************
*********************************************************************/
void timer1_clear(void)
{
	nrfx_timer_clear(&TIMER_INSTANCE);   // ���������
}
/*******************************************************************
********************************************************************
* ���ƣ�void timer_stop_and_clear(void)
* ���ܣ� ��ռ�������ֹͣ
* ������
* ����ֵ��
* �汾��v1
* ���ߣ�matt
* ���ڣ�2026-06-06
* ������
* ��ע��
* 
* 
*********************************************************************
*********************************************************************/
void timer1_stop_and_clear(void)
{
	nrfx_timer_disable(&TIMER_INSTANCE);  // ��ֹͣ��ʱ��
	nrfx_timer_clear(&TIMER_INSTANCE);   // ���������
}
/*******************************************************************
********************************************************************
* ���ƣ�
* ���ܣ���ȡ����Ĵ������Ե�ַ
* ������task : ����ö�٣��� NRF_TIMER_TASK_CLEAR
* ����ֵ��
* �汾��v1
* ���ߣ�matt
* ���ڣ�2026-06-06
* ������
* ��ע��
* 
* 
*********************************************************************
*********************************************************************/
uint32_t timer1_task_addr_get(nrf_timer_task_t task)
{
    /* nrfx ��װ����ͨ�� HAL ȡָ�룬��ת uint32_t */
    return nrfx_timer_task_address_get(&TIMER_INSTANCE, task);
}
/*******************************************************************
********************************************************************
* ���ƣ�
* ���ܣ�
* ������
* ����ֵ��
* �汾��v1
* ���ߣ�matt
* ���ڣ�2026-06-06
* ������
* ��ע��
* 
* 
*********************************************************************
*********************************************************************/
void timer1_irq_handler(void)
{
    /* ת�� nrfx_timer �ڲ� irq_handler()��
     * ��������� COMPARE ͨ��������¼����ص��û� handler */
    nrfx_timer_1_irq_handler();
}

