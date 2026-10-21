#include "systick_delay_freertos.h"
#include "stm32f4xx.h"
#include "FreeRTOS.h"                           // 使用freertos 操作系统时候包含头文件
#include "task.h"
//===========================================================================
static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在freertos下,代表每个节拍的ms数

/*******************************************************************
********************************************************************
* 名称： void SysTick_Handler(void)
* 功能：  滴答定时器中断函数
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
extern void xPortSysTickHandler(void);
void SysTick_Handler(void)
{
	if(xTaskGetSchedulerState()!= taskSCHEDULER_NOT_STARTED)//系统已经运行
	{
		xPortSysTickHandler();
	}
}


//extern void xPortSysTickHandler(void);
//void SysTick_Handler(void)
//{
//    #if (INCLUDE_xTaskGetSchedulerState  == 1 )
//      if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
//      {
//    #endif  /* INCLUDE_xTaskGetSchedulerState */  
//        xPortSysTickHandler();
//    #if (INCLUDE_xTaskGetSchedulerState  == 1 )
//      }
//    #endif  /* INCLUDE_xTaskGetSchedulerState */

//}
/*******************************************************************
********************************************************************
* 名称：void systick_delay__freerots_init()
* 功能：滴答定时器在使用free rtos的时候的初始化函数
* 参数： 无
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2022-09-05
* 举例：
* 备注：这里为了兼容 FreeRTOS，所以将 SYSTICK 的时钟频率改为 AHB 的频率！
* SYSTICK 的时钟固定为 AHB 时钟，基础例程里面 SYSTICK 时钟频率为 AHB/8
* 
*********************************************************************
*********************************************************************/
void systick_delay_freerots_init(u8 SYSCLK)
{
	u32 reload;
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK); 
	fac_us=SYSCLK;							//不论是否使用OS,fac_us都需要使用
	reload=SYSCLK;							//每秒钟的计数次数 单位为M	   
	reload*=1000000/configTICK_RATE_HZ;		//根据configTICK_RATE_HZ设定溢出时间
											//reload为24位寄存器,最大值:16777216,在168M下,约合0.0998s左右	
	fac_ms=1000/configTICK_RATE_HZ;			//代表OS可以延时的最少单位	   
	SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;//开启SYSTICK中断
	SysTick->LOAD=reload; 					//每1/configTICK_RATE_HZ断一次	
	SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk; //开启SYSTICK    
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
//延时nus
//nus:要延时的us数.	
//nus:0~204522252(最大值即2^32/fac_us@fac_us=168)	    								   
void systick_delay_us(u32 nus)
{		
	u32 ticks = 0;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus*fac_us; 						//需要的节拍数 
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	}										    
}
/*******************************************************************
********************************************************************
* 名称：void systick_delay_freerots_ms(u32 nms)
* 功能：延时nms (引发调度)
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
//
//nms:要延时的ms数
//nms:0~65535
void systick_delay_freerots_ms(u32 nms)
{	
	if(xTaskGetSchedulerState()!=taskSCHEDULER_NOT_STARTED)//系统已经运行
	{		
		if(nms>=fac_ms)						//延时的时间大于OS的最少时间周期 
		{ 
   			vTaskDelay(nms/fac_ms);	 		//FreeRTOS延时
		}
		nms%=fac_ms;						//OS已经无法提供这么小的延时了,采用普通方式延时    
	}
	systick_delay_us((u32)(nms*1000));				//普通方式延时
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
//延时nms,不会引起任务调度
//nms:要延时的ms数
void systick_delay_ms(u32 nms)
{
	u32 i;
	for(i=0;i<nms;i++) 
	{
		systick_delay_us(1000);
	}
	
}




