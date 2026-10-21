#include "gpiote.h"
#include "sdk_config.h"
#include "nrf_drv_gpiote.h"
#include "led.h"
#include "button.h"
//=======================================================================================

/*******************************************************************
********************************************************************
* 名称：void gpiote_module_enable(void)
* 功能： 使能GPIOTE 引脚
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void gpiote_module_enable(void)
{
	ret_code_t err_code;	
	//1 : 初始化GPIOTE程序模块
	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);

}


/*******************************************************************
********************************************************************
* 名称：void  gpiote_pin_out_task_init(uint8_t pin_num)
* 功能： 初始化引脚为输出任务引脚+ 使能任务模式
* 参数：pin_num   0-31  引脚号
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void  gpiote_pin_out_task_init(uint8_t pin_num)
{
	ret_code_t err_code;	
	//2 :定义GPIOTE输出初始化结构体，并对其成员变量赋值
	nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_TASK_TOGGLE(true);
  
	//3 ：初始化GPIOTE输出引脚
	err_code = nrf_drv_gpiote_out_init(pin_num, &out_config);
	APP_ERROR_CHECK(err_code);
	
    //4 ：使能引脚pin_num 所在GPIOTE通道的任务模式，一旦使能任务模式后，只能用GPIOTE操作该引脚
	nrf_drv_gpiote_out_task_enable(pin_num);
}

/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void in_pin_handler(nrf_drv_gpiote_pin_t pin , nrf_gpiote_polarity_t action)
{
	if(pin == BUTTON_13) // 判断引脚
	{
	
	}
	//判断引脚动作
	if(action == NRF_GPIOTE_POLARITY_HITOLO) // 电平由高到低
	{
		nrf_gpio_pin_toggle(LED_17);
	}
	else if(action == NRF_GPIOTE_POLARITY_LOTOHI) // 电平由低到高
	{
		nrf_gpio_pin_toggle(LED_18);
	}
	else if(action == NRF_GPIOTE_POLARITY_TOGGLE)  // 边缘双触发
	{
		nrf_gpio_pin_toggle(LED_19);
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
* 日期：2026-06-06
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void gpiote_pin_in_event_init(int8_t pin_num)
{
	ret_code_t err_code;	
	nrf_drv_gpiote_in_config_t  in_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true); //高电平到低电平变化产生事件，即下降沿产生事件
	in_config.pull = NRF_GPIO_PIN_PULLUP;                                        // 配置为上拉
	err_code = nrf_drv_gpiote_in_init(pin_num, &in_config, in_pin_handler);      // 配置pin_num 引脚为GPIOTE 输入
	APP_ERROR_CHECK(err_code);
	
	nrf_drv_gpiote_in_event_enable(pin_num, true);  // 使能引脚所在的GPIOTE 通道事件模式

}




