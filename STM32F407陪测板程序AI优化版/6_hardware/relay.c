#include "relay.h"
#include "adc.h"
#include  "stm32f4xx.h" 
#include  "stdbool.h"
#include "usart_dma_all.h"

#include "arithmetic.h"
#include "systick_delay_freertos.h"
#include "lcd.h"
#include "variable_global.h"
#include <string.h>        // 使用字符串处理函数
#include <stdio.h>         //使用sprintf函数时包含
#include "stdlib.h"
#include "w25q32.h"
#include "systick_delay_freertos.h"
//=======================================================================================
// u8 multiplier_global = 1 ;  // 运放的放大倍数全局变量
int16_t save_adc_data_global[256] = {0} ; // 用于保存stm32从adc中读取的多次数据
u8  test_level_select_falg = 0  ;        // 用于标记量程切换的标记控制
/*******************************************************************
********************************************************************
* 名称：void relay_init(void)
* 功能： 初始化控制继电器的8个pin脚为推完输出，且输出为低电平
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
H : 导通
L ：不导通
PC11 : k1
PC12 : k2
  
  
  
* 
*********************************************************************
*********************************************************************/
void relay_init(void)
{
	GPIO_InitTypeDef    gpio_init_gpio ;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIO C 时钟
	// PC11   推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_11;
	gpio_init_gpio.GPIO_Mode = GPIO_Mode_OUT;  //普通输出模式
	gpio_init_gpio.GPIO_OType = GPIO_OType_PP;//推挽输出
	gpio_init_gpio.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	gpio_init_gpio.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
	GPIO_Init(GPIOC,&gpio_init_gpio);//初始化gpio口为推挽输出
	
	// PC12    推挽输出
	gpio_init_gpio.GPIO_Pin = GPIO_Pin_12;
	gpio_init_gpio.GPIO_Mode = GPIO_Mode_OUT;  //普通输出模式
	gpio_init_gpio.GPIO_OType = GPIO_OType_PP;//推挽输出
	gpio_init_gpio.GPIO_Speed = GPIO_Speed_100MHz; //100MHz
	gpio_init_gpio.GPIO_PuPd = GPIO_PuPd_DOWN;//下拉
	GPIO_Init(GPIOC,&gpio_init_gpio);//初始化gpio口为推挽输出
	

	//	将继电器关闭
	GPIO_ResetBits(GPIOC,GPIO_Pin_3); // PC11
	GPIO_ResetBits(GPIOD,GPIO_Pin_3); // PC12
	
	
}

/*******************************************************************
********************************************************************
* 名称：void relay_ctrl(u8 channel_num,bool on_off)
* 功能：通过选择的通道及量程，切换对应的继电器
* 参数：channel_num：通道，有1---4个通道  (每个通道各对应 4个ADC)
        level_select ：电流量程选择， 高 ，中， 低
   
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2023-06-13
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void relay_ctrl(u8 channel_num,u8 level_select)
{
	if(channel_num == CH1 ) // CH1 
	{
		if(level_select == H )       // 高电流档位测试 1MA---800MA
		{
			RELAY_K1 = 1 ;
			RELAY_K2 = 0 ;
			// multiplier_global = 200 ;   //  电流放大系数
			usart1_printf("1\r\n");
		}
		else if(level_select == M )  //  中 电流档位测试 1ua----60mA
		{
			RELAY_K1 = 0 ;
			RELAY_K2 = 0 ;
			// multiplier_global = 10 ;   //  电流放大系数
			usart1_printf("2\r\n");
		}
		else if(level_select == L )    //  低 电流档位测试  0.02ua---1mA  
		{
			RELAY_K1 = 0 ;
			RELAY_K2 = 1 ;
			// multiplier_global = 10 ;   //  电流放大系数
			usart1_printf("3\r\n");
		}	
	}
	

}
/*******************************************************************
********************************************************************
* int16_t get_adc0_average_current(u8 read_num,u8 delete_num)
* 功能：读取 N次ADC 值，保存在全局变量中，并排序后，去掉前后无效值，返回剩余数据的平均值（电压值），并计算出此值表示的电流值（A）
* 参数：read_num ： 要读取数据的个数
       delete_num ：要去掉前后无效数据的个数 
* 返回值：有效数据的平均值,所对应的ADC电压值
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
*   adc_read_data【0】 ： 保存PA0 读到的数据 0.02  *200  
	adc_read_data【1】 ： 保存PA1 
	adc_read_data【2】 ： 保存PA2   
	adc_read_data【3】 ： 保存PC3 
*   4096: 12BIT 的adc， 2的12次方 = 4096
3.3 ：因为参考电压是 3.3v
*********************************************************************
*********************************************************************/
// static double get_adc0_average_current(u8 read_num,u8 delete_num)
// {
// 	uint16_t voltage_value = 0; // 用于保存采集到的平均电压值
// 	double current_value = 0.0  ;
// 	if(read_num % 2 == 0) // 要处理的数据为偶数时
// 	{
// 		if(delete_num < ( read_num / 2 ))
// 		{ 
// 			// 将save_adc_data_global 全部清理，方便后续保存数据
// 			memset(save_adc_data_global,0,strlen((const char*)save_adc_data_global));
// 			for(u8 i = 0 ;i < read_num; i++)
// 			{
// 				// 从adc0 中读取 read_num 次数据，并保存在save_adc_data_global 中
// 				save_adc_data_global [i] = adc_read_data[0] ; 
// 				// usart1_printf("save_adc_data_global [%d] = %d \r\n",i,save_adc_data_global [i]);
// 			}
// 			// usart1_printf("value1 = %lf\r\n", (double)get_average(save_adc_data_global,read_num,delete_num));
// 			// 将保存在save_adc_data_global 中的数据，排序，去前后无效值，取平均,并计算出ADC0 的电流值 （A）
// 			voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
// 			usart1_printf("MAX LEVEL,VOLTAGE = %d\r\n",voltage_value); //打印本次采集到的的平均电压值
// 			current_value = (double)( (double)voltage_value / 4096  * 3.3 / 200 / 0.02 );
// 			usart1_printf("MAX LEVEL,CURRENT = %.09lf A\r\n",current_value); //打印本次采集到的的平均电流值
// 			return current_value ;
// 		}
// 		else
// 		{
// 			usart1_printf("The delete_num is too large1\r\n"); 
// 			return 0.0 ;
// 		}
// 	}
// 	else  // 要处理的数据时奇数时
// 	{
// 		if(delete_num <= ( read_num / 2 ))
// 		{
// 			// 将save_adc_data_global 全部清理，方便后续保存数据
// 			memset((void*)save_adc_data_global,0,strlen((const char*)save_adc_data_global));
// 			for(u8 i = 0 ;i < read_num; i++)
// 			{
// 				// 从adc0 中读取 read_num 次数据，并保存在save_adc_data_global 中
// 				save_adc_data_global [i] = adc_read_data[0] ; 
// 			}
// 			// 将保存在save_adc_data_global 中的数据，排序，去前后无效值，取平均,并计算出ADC0 的电流值 （A）
// 			voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
// 			usart1_printf("MAX LEVEL,VOLTAGE = %d\r\n",voltage_value); //打印本次采集到的的平均电压值
// 			current_value = (double)( (double)voltage_value / 4096  * 3.3 / 200 / 0.02 );
// 			usart1_printf("MAX LEVEL,CURRENT = %.09lf A\r\n",current_value); //打印本次采集到的的平均电流值
// 			return current_value ;
// 		}
// 		else
// 		{
// 			usart1_printf("The delete_num is too large2\r\n"); 
// 			return 0.0 ;
// 		}
// 	}

// }
/*******************************************************************
********************************************************************
* 名称：int16_t auto_get_adc_average_current(u8 read_num,u8 delete_num,u8 adc_select,u8 shunt_resistor_select)
* 功能：按照读取次数，删除无效数据后，取平均后，转换的电流值（单位A）- 0点校准值
* 参数：
read_num   :要读取的数据个数
delete_num ：前后无效数据要删除的个数
adc_select ：选择的adc 端口 （PA0 PA1 PA2 PC3）
shunt_resistor_select :检流电阻选择 （0.02，  5  ，300）
* 返回值：电流值（单位A）
* 版本：v1
* 作者：matt
* 日期：2025-02-16
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

double auto_get_adc_average_current(u8 read_num,u8 delete_num,u8 adc_select,u8 shunt_resistor_select)
{
	int16_t voltage_value = 0 ; // 保存电压值
	u8 current_buf[16] = {0}; // 保存电流值字符串
	double current_value = 0.0 ; // 保存电流值
	double zero_current = 0 ;  // 保存从flash 中读取到的0点校准值
	if(read_num % 2 == 0) // 要处理的数据时偶数时
	{
		if(delete_num < ( read_num / 2 ))
		{ 
			// 将save_adc_data_global 全部清理，方便后续保存数据
			memset(save_adc_data_global,0,strlen((const char*)save_adc_data_global));
			for(u8 i = 0 ;i < read_num; i++)
			{
				// 从adc 中读取 read_num 次数据，并保存在save_adc_data_global 中
				if(adc_select == PA0_0)
				{
					save_adc_data_global [i] = adc_read_data[0] ; // 保存PA0_0 的采集数据
					usart1_printf("[ PA0==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PA1_1)
				{
					save_adc_data_global [i] = adc_read_data[1] ; // 保存PA1_1 的采集数据
					usart1_printf("[ PA1==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select  == PA2_2)
				{
					save_adc_data_global [i] = adc_read_data[2] ; // 保存PA2_2 的采集数据
					usart1_printf("[ PA2==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PC3_13)
				{
					save_adc_data_global [i] = adc_read_data[3] ; // 保存PC3_13 的采集数据
					usart1_printf("[ PC3==%d== ] \r\n",save_adc_data_global [i]);
				}
				
			}

			if((shunt_resistor_select == SHUNT_RESISTOR_0_02R) && (adc_select == PA0_0))// 采用0.02采样电阻 PA0
			{
				// 将保存在save_adc_data_global 中的数据，排序，去前后无效值，取平均,并计算出PA0 的电流值 （单位A）
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 200 / 0.02 ); // 电压换算为电流A 
				usart1_printf("CURRENT = %.09lf[ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",current_value  );

				w25q32_flash_read(current_buf,0x0000002d,15,1) ;   // 读取指定位置指定长度的电流数据（0点校准值），保存在current_buf
				zero_current = atof((const char *)current_buf ) ;  //将电流字符串转换为double 数据
				if(current_value > zero_current )
				{
					current_value -= zero_current ; // 转换后的电流值 - 0点校准后的值
				}
				else
				{
					current_value = 0 ; // 当测试到的数据比0点校准值还小，就显示0
				}
				usart1_printf("RETURN VALUE = %.09lf[ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",current_value  );
				return current_value ;
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA1_1)) // 5R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",current_value);

				w25q32_flash_read(current_buf,0x0000003c,15,1);   // 读取指定位置指定长度的电流数据（0点校准值），保存在current_buf
				zero_current = atof((const char *)current_buf ) ; //将电流字符串转换为double 数据
				if(current_value > zero_current )
				{
					current_value -= zero_current ; // 转换后的电流值 - 0点校准后的值
				}
				else
				{
					current_value = 0 ; // 当测试到的数据比0点校准值还小，就显示0
				}
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PC3_13)) // 5R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 2 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",current_value);

				w25q32_flash_read(current_buf, 0x0000004B,15,1);   // 读取指定位置指定长度的电流数据（0点校准值），保存在current_buf
				zero_current = atof((const char *)current_buf ) ; //将电流字符串转换为double 数据
				if(current_value > zero_current )
				{
					current_value -= zero_current ; // 转换后的电流值 - 0点校准后的值
				}
				else
				{
					current_value = 0 ; // 当测试到的数据比0点校准值还小，就显示0
				}
				return current_value * 1.007461569;
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA2_2)) // 5R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value  / 4095  * 3.3 / 10 / 63.5 / 5);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",current_value);

				w25q32_flash_read(current_buf, 0x0000005A,15,1);   // 读取指定位置指定长度的电流数据（0点校准值），保存在current_buf
				zero_current = atof((const char *)current_buf ) ; //将电流字符串转换为double 数据
				if(current_value > zero_current )
				{
					current_value -= zero_current ; // 转换后的电流值 - 0点校准后的值
				}
				else
				{
					current_value = 0 ; // 当测试到的数据比0点校准值还小，就显示0
				}
				return current_value ;
				
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA1_1)) // 300R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095 * 3.3 / 10  / 300);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",current_value);

				w25q32_flash_read(current_buf, 0x00000069,15,1);   // 读取指定位置指定长度的电流数据（0点校准值），保存在current_buf
				zero_current = atof((const char *)current_buf ) ;  //将电流字符串转换为double 数据
				if(current_value > zero_current )
				{
					current_value -= zero_current ; // 转换后的电流值 - 0点校准后的值
				}
				else
				{
					current_value = 0 ; // 当测试到的数据比0点校准值还小，就显示0
				}
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PC3_13)) // 300R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /2 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",current_value);

				w25q32_flash_read(current_buf, 0x00000078,15,1);   // 读取指定位置指定长度的电流数据（0点校准值），保存在current_buf
				zero_current = atof((const char *)current_buf ) ;  //将电流字符串转换为double 数据
				if(current_value > zero_current )
				{
					current_value -= zero_current ; // 转换后的电流值 - 0点校准后的值
				}
				else
				{
					current_value = 0 ; // 当测试到的数据比0点校准值还小，就显示0
				}
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA2_2)) // 300R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /63.5 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",current_value);

				w25q32_flash_read(current_buf, 0x00000087,15,1);   // 读取指定位置指定长度的电流数据（0点校准值），保存在current_buf
				zero_current = atof((const char *)current_buf ) ;  //将电流字符串转换为double 数据
				if(current_value > zero_current )
				{
					current_value -= zero_current ; // 转换后的电流值 - 0点校准后的值
				}
				else
				{
					current_value = 0 ; // 当测试到的数据比0点校准值还小，就显示0
				}
				return current_value ;
			}
			else
			{
				usart1_printf("The parameter error3\r\n"); 
				return 0.0 ;
			}
			
		}
		else
		{
			usart1_printf("The delete_num is too large4\r\n"); 
			return 0.0 ;
		}
	}
	else  // 要处理的数据时奇数时
	{
		if(delete_num <= ( read_num / 2 ))
		{
			// 将save_adc_data_global 全部清理，方便后续保存数据
			memset(save_adc_data_global,0,strlen((const char*)save_adc_data_global));
			for(u8 i = 0 ;i < read_num; i++)
			{
				// 从adc 中读取 read_num 次数据，并保存在save_adc_data_global 中
				if(adc_select == PA0_0)
				{
					save_adc_data_global [i] = adc_read_data[0] ; // 保存PA0_0 的采集数据
					usart1_printf("[ PA0==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PA1_1)
				{
					save_adc_data_global [i] = adc_read_data[1] ; // 保存PA1_1 的采集数据
					usart1_printf("[ PA1==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select  == PA2_2)
				{
					save_adc_data_global [i] = adc_read_data[2] ; // 保存PA2_2 的采集数据
					usart1_printf("[ PA2==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PC3_13)
				{
					save_adc_data_global [i] = adc_read_data[3] ; // 保存PC3_13 的采集数据
					usart1_printf("[ PC3==%d== ] \r\n",save_adc_data_global [i]);
				}
				
			}

			if((shunt_resistor_select == SHUNT_RESISTOR_0_02R) && (adc_select == PA0_0))// 采用0.02采样电阻 PA0
			{
				// 将保存在save_adc_data_global 中的数据，排序，去前后无效值，取平均,并计算出PA0 的电流值 （单位A）
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 200 / 0.02 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",current_value);
				return current_value ;
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA1_1)) // 5R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",current_value);
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PC3_13)) // 5R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 2 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",current_value);
				return current_value ;
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA2_2)) // 5R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value  / 4095  * 3.3 / 10 / 63.5 / 5);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",current_value);
				return current_value ;
				
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA1_1)) // 300R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095 * 3.3 / 10  / 300);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",current_value);
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PC3_13)) // 300R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /2 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",current_value);
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA2_2)) // 300R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /63.5 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",current_value);
				return current_value ;
			}
			else
			{
				usart1_printf("The parameter error5\r\n"); 
				return 0.0 ;
			}
			
		}
		else
		{
			usart1_printf("The delete_num is too large6\r\n"); 
			return 0.0 ;
		}
	}

}
/*******************************************************************
********************************************************************
* 名称：static double auto_get_adc_average_current_for_calibration(u8 read_num,u8 delete_num,u8 adc_select,u8 shunt_resistor_select)
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2025-02-16
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
static double auto_get_adc_average_current_for_calibration(u8 read_num,u8 delete_num,u8 adc_select,u8 shunt_resistor_select)
{
	int16_t voltage_value = 0 ; // 保存电压值
	// u8 current_buf[16] = {0}; // 保存电流值字符串
	double current_value = 0.0 ; // 保存电流值
	// double zero_current = 0 ;  // 保存从flash 中读取到的0点校准值
	if(read_num % 2 == 0) // 要处理的数据时偶数时
	{
		if(delete_num < ( read_num / 2 ))
		{ 
			// 将save_adc_data_global 全部清理，方便后续保存数据
			memset(save_adc_data_global,0,strlen((const char*)save_adc_data_global));
			for(u8 i = 0 ;i < read_num; i++)
			{
				// 从adc 中读取 read_num 次数据，并保存在save_adc_data_global 中
				if(adc_select == PA0_0)
				{
					save_adc_data_global [i] = adc_read_data[0] ; // 保存PA0_0 的采集数据
					// usart1_printf("[ PA0==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PA1_1)
				{
					save_adc_data_global [i] = adc_read_data[1] ; // 保存PA1_1 的采集数据
					// usart1_printf("[ PA1==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select  == PA2_2)
				{
					save_adc_data_global [i] = adc_read_data[2] ; // 保存PA2_2 的采集数据
					// usart1_printf("[ PA2==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PC3_13)
				{
					save_adc_data_global [i] = adc_read_data[3] ; // 保存PC3_13 的采集数据
					// usart1_printf("[ PC3==%d== ] \r\n",save_adc_data_global [i]);
				}
				
			}

			if((shunt_resistor_select == SHUNT_RESISTOR_0_02R) && (adc_select == PA0_0))// 采用0.02采样电阻 PA0
			{
				// 将保存在save_adc_data_global 中的数据，排序，去前后无效值，取平均,并计算出PA0 的电流值 （单位A）
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 200 / 0.02 ); // 电压换算为电流A 
				usart1_printf("CURRENT = %.09lf[ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",current_value  );

				return current_value ;
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA1_1)) // 5R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",current_value);

				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PC3_13)) // 5R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 2 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",current_value);

				return current_value ;
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA2_2)) // 5R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value  / 4095  * 3.3 / 10 / 63.5 / 5);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",current_value);

				return current_value ;
				
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA1_1)) // 300R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095 * 3.3 / 10  / 300);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",current_value);

				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PC3_13)) // 300R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /2 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",current_value);

				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA2_2)) // 300R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /63.5 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",current_value);

				return current_value ;
			}
			else
			{
				usart1_printf("The parameter error3\r\n"); 
				return 0.0 ;
			}
			
		}
		else
		{
			usart1_printf("The delete_num is too large4\r\n"); 
			return 0.0 ;
		}
	}
	else  // 要处理的数据时奇数时
	{
		if(delete_num <= ( read_num / 2 ))
		{
			// 将save_adc_data_global 全部清理，方便后续保存数据
			memset(save_adc_data_global,0,strlen((const char*)save_adc_data_global));
			for(u8 i = 0 ;i < read_num; i++)
			{
				// 从adc 中读取 read_num 次数据，并保存在save_adc_data_global 中
				if(adc_select == PA0_0)
				{
					save_adc_data_global [i] = adc_read_data[0] ; // 保存PA0_0 的采集数据
					// usart1_printf("[ PA0==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PA1_1)
				{
					save_adc_data_global [i] = adc_read_data[1] ; // 保存PA1_1 的采集数据
					// usart1_printf("[ PA1==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select  == PA2_2)
				{
					save_adc_data_global [i] = adc_read_data[2] ; // 保存PA2_2 的采集数据
					// usart1_printf("[ PA2==%d== ] \r\n",save_adc_data_global [i]);
				}
				else if(adc_select == PC3_13)
				{
					save_adc_data_global [i] = adc_read_data[3] ; // 保存PC3_13 的采集数据
					// usart1_printf("[ PC3==%d== ] \r\n",save_adc_data_global [i]);
				}
				
			}

			if((shunt_resistor_select == SHUNT_RESISTOR_0_02R) && (adc_select == PA0_0))// 采用0.02采样电阻 PA0
			{
				// 将保存在save_adc_data_global 中的数据，排序，去前后无效值，取平均,并计算出PA0 的电流值 （单位A）
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 200 / 0.02 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_0_02R , PA0_0 ]\r\n",current_value);
				return current_value ;
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA1_1)) // 5R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA1_1 ]\r\n",current_value);
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PC3_13)) // 5R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 / 2 / 5 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PC3_13 ]\r\n",current_value);
				return current_value ;
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_5R) && (adc_select == PA2_2)) // 5R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value  / 4095  * 3.3 / 10 / 63.5 / 5);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_5R , PA2_2 ]\r\n",current_value);
				return current_value ;
				
			}

			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA1_1)) // 300R 采样电阻， PA1
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095 * 3.3 / 10  / 300);
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA1_1 ]\r\n",current_value);
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PC3_13)) // 300R 采样电阻， PC3
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /2 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PC3_13 ]\r\n",current_value);
				return current_value ;
				
			}
			else if((shunt_resistor_select == SHUNT_RESISTOR_300R) && (adc_select == PA2_2)) // 300R 采样电阻， PA2
			{
				voltage_value = get_average(save_adc_data_global,read_num,delete_num) ;
				usart1_printf("VOLTAGE = %d [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",voltage_value);

				current_value = (double)((double)voltage_value / 4095  * 3.3 / 10 /63.5 / 300 );
				usart1_printf("CURRENT = %.09lf [ SHUNT_RESISTOR_300R , PA2_2 ]\r\n",current_value);
				return current_value ;
			}
			else
			{
				usart1_printf("The parameter error5\r\n"); 
				return 0.0 ;
			}
			
		}
		else
		{
			usart1_printf("The delete_num is too large6\r\n"); 
			return 0.0 ;
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
* 日期：2025-02-16
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

void zero_calibration(u8 read_num,u8 delete_num)
{
	double current_test_value = 0 ;  // 用于保存读取到的电流值

	relay_ctrl(CH1, H) ;  // 默认使用最大量程来测试 （PA0 0.02 200）
	systick_delay_ms(20); // 延时，保证继电器切换稳定
	current_test_value = auto_get_adc_average_current_for_calibration( read_num, delete_num,PA0_0,SHUNT_RESISTOR_0_02R);  // 0.02R  PA0   
	memset((void*)current_A_string_buf,0,sizeof((const char*)current_A_string_buf)); // 清空current_A_string_buf
	sprintf((char *)current_A_string_buf,"%.09lf",current_test_value); //  将读取到的电流值按照LF的格式打印到字符数组中
	usart1_printf("calibration [0.02 PA0] %s\r\n",current_A_string_buf);
	display_ascii_string(6,60,8,(u8*)current_A_string_buf,Blue,Black);
	// 将电流值A的大小 保存进flash ，从 0x0000002d开始，的15个字节
	w25q32_flash_autosector_write_with_erase((u8*) current_A_string_buf,0x0000002d,15,1);  

	
	relay_ctrl(CH1, M);  // 切换档位，使用 5R   10  PA1
	systick_delay_ms(20);// 延时，保证继电器切换稳
	current_test_value = auto_get_adc_average_current_for_calibration( read_num, delete_num,PA1_1,SHUNT_RESISTOR_5R); // 5R  PA1
	memset((void*)current_A_string_buf,0,sizeof((const char*)current_A_string_buf)); // 清空current_A_string_buf
	sprintf((char *)current_A_string_buf,"%.09lf",current_test_value); //  将读取到的电流值按照LF的格式打印到字符数组中
	usart1_printf("calibration [5 PA1] %s\r\n",current_A_string_buf);
	display_ascii_string(6,80,8,(u8*)current_A_string_buf,Blue,Black);
	// 将电流值A的大小 保存进flash ，从 0x0000003c开始，的15个字节
	w25q32_flash_autosector_write_with_erase((u8*) current_A_string_buf,0x0000003C,15,1);  

	current_test_value = auto_get_adc_average_current_for_calibration( read_num, delete_num,PC3_13,SHUNT_RESISTOR_5R); // 5R 采样电阻， PC3
	memset((void*)current_A_string_buf,0,sizeof((const char*)current_A_string_buf)); // 清空current_A_string_buf
	sprintf((char *)current_A_string_buf,"%.09lf",current_test_value); //  将读取到的电流值按照LF的格式打印到字符数组中
	usart1_printf("calibration [5 PC3] %s\r\n",current_A_string_buf);
	display_ascii_string(6,100,8,(u8*)current_A_string_buf,Blue,Black);
	// 将电流值A的大小 保存进flash ，从 0x0000004B开始，的15个字节
	w25q32_flash_autosector_write_with_erase((u8*) current_A_string_buf,0x0000004B,15,1); 

	current_test_value = auto_get_adc_average_current_for_calibration( read_num, delete_num,PA2_2,SHUNT_RESISTOR_5R); // 5R  PA2
	memset((void*)current_A_string_buf,0,sizeof((const char*)current_A_string_buf)); // 清空current_A_string_buf
	sprintf((char *)current_A_string_buf,"%.09lf",current_test_value); //  将读取到的电流值按照LF的格式打印到字符数组中
	usart1_printf("calibration [5 PA2] %s\r\n",current_A_string_buf);
	display_ascii_string(6,120,8,(u8*)current_A_string_buf,Blue,Black);
	// 将电流值A的大小 保存进flash ，从 0x0000005A开始，的15个字节
	w25q32_flash_autosector_write_with_erase((u8*) current_A_string_buf,0x0000005A,15,1); 


	
	relay_ctrl(CH1, L);  // 切换档位，使用 300R   10*  PA1
	systick_delay_ms(20);// 延时，保证继电器切换稳定
	current_test_value = auto_get_adc_average_current_for_calibration( read_num, delete_num,PA1_1,SHUNT_RESISTOR_300R);  // 300R 采样电阻， PA1
	memset((void*)current_A_string_buf,0,sizeof((const char*)current_A_string_buf)); // 清空current_A_string_buf
	sprintf((char *)current_A_string_buf,"%.09lf",current_test_value); //  将读取到的电流值按照LF的格式打印到字符数组中
	usart1_printf("calibration [300 PA1] %s\r\n",current_A_string_buf);
	display_ascii_string(6,140,8,(u8*)current_A_string_buf,Blue,Black);
	// 将电流值A的大小 保存进flash ，从 0x00000069开始，的15个字节
	w25q32_flash_autosector_write_with_erase((u8*) current_A_string_buf,0x00000069,15,1);

	current_test_value = auto_get_adc_average_current_for_calibration( read_num, delete_num,PC3_13,SHUNT_RESISTOR_300R); // 300R 采样电阻， PC3
	memset((void*)current_A_string_buf,0,sizeof((const char*)current_A_string_buf)); // 清空current_A_string_buf
	sprintf((char *)current_A_string_buf,"%.09lf",current_test_value); //  将读取到的电流值按照LF的格式打印到字符数组中
	usart1_printf("calibration [300 PC3] %s\r\n",current_A_string_buf);
	display_ascii_string(6,160,8,(u8*)current_A_string_buf,Blue,Black);
	// 将电流值A的大小 保存进flash ，从 0x00000078开始，的15个字节
	w25q32_flash_autosector_write_with_erase((u8*) current_A_string_buf,0x00000078,15,1);

	current_test_value = auto_get_adc_average_current_for_calibration( read_num, delete_num,PA2_2,SHUNT_RESISTOR_300R); // 300R 采样电阻， PA2
	memset((void*)current_A_string_buf,0,sizeof((const char*)current_A_string_buf)); // 清空current_A_string_buf
	sprintf((char *)current_A_string_buf,"%.09lf",current_test_value); //  将读取到的电流值按照LF的格式打印到字符数组中
	usart1_printf("calibration [300 PA2] %s\r\n",current_A_string_buf);
	display_ascii_string(6,180,8,(u8*)current_A_string_buf,Blue,Black);
	// 将电流值A的大小 保存进flash ，从 0x00000087开始，的15个字节
	w25q32_flash_autosector_write_with_erase((u8*) current_A_string_buf,0x00000087,15,1);

}
/*******************************************************************
********************************************************************
* 名称：int16_t get_average_current(u8 read_num,u8 delete_num)
* 功能：返回指定的read_num 个数据中，去掉收尾delete_num个无效数据后，转换后的电流值（单位A）
* 参数：
read_num : 读取的次数
delete_num ：收尾无效数据的个数
* 返回值 ： 电流值
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
*   adc_read_data【0】 ： 保存PA0 读到的数据 0.02  *200  
	adc_read_data【1】 ： 保存PA1 
	adc_read_data【2】 ： 保存PA2   
	adc_read_data【3】 ： 保存PC3  
*********************************************************************
*********************************************************************/
// static double get_average_current(u8 read_num,u8 delete_num)
// {
// 	double read_current = 0.0 ;
// 	if(range_changeover_times_global == 0) // 第一次切换
// 	{
// 		range_changeover_times_global = 1 ; // 最大量程测试一次后，不用再切换
// 		relay_ctrl(CH1, H) ;  // 默认使用最大量程来测试 （PA0 0.02 200）
// 		systick_delay_ms(20); // 延时，保证继电器切换稳定
// 		read_current = get_adc0_average_current(read_num,delete_num) ; // 用最大量程读取一次电流数据
// 	}
	
	
// 	if(read_current >= 0.8) // 大于0.8A, 超过最大量程
// 	{
// 		// 停止测试 (超过量程)
// 		return -1.0 ;
// 	}
// 	else if(read_current >= 0.066) //  0.066A  <= x  < 0.8A   
// 	{
// 		// 不用切换档位
// 		 return auto_get_adc_average_current(read_num,delete_num,PA0_0,SHUNT_RESISTOR_0_02R) ;

// 	}
// 	else if(read_current >= 0.033) // 0.033A <=  X  < 0.066A
// 	{
// 		// 切换档位，使用 5R   10  PA1
// 		relay_ctrl(CH1, M);
// 		systick_delay_ms(20);// 延时，保证继电器切换稳定
// 		return auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_5R) ;
// 	}
// 	else if(read_current >= 0.001) // 0.001A <=  X  < 0.033A
// 	{
// 		// 切换档位，使用 5R   10*2  PC3
// 		relay_ctrl(CH1, M);
// 		systick_delay_ms(20);// 延时，保证继电器切换稳定
// 		return auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_5R) ;

// 	}
// 	else if(read_current >= 0.000055) // 0.000055A <=  X  < 0.001A
// 	{
// 		// 切换档位，使用 300R   10*  PA1
// 		relay_ctrl(CH1, L);
// 		systick_delay_ms(20);// 延时，保证继电器切换稳定
// 		return auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_300R) ;

// 	}
// 	else if(read_current >= 0.0000001) // 0.0000001A <=  X  < 0.000055A
// 	{
// 		// 切换档位，使用 300R   10* 2  PC3
// 		relay_ctrl(CH1, L);
// 		systick_delay_ms(20);// 延时，保证继电器切换稳定
// 		return auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_300R) ;

// 	}
// 	else //  <  0.0000001A
// 	{
// 		// 切换档位，使用 300R   10*6.35  PA2
// 		relay_ctrl(CH1, L);
// 		systick_delay_ms(20);// 延时，保证继电器切换稳定
// 		return auto_get_adc_average_current(read_num,delete_num,PA2_2,SHUNT_RESISTOR_300R) ;
		

// 	}
// }
/*******************************************************************
********************************************************************
* 名称：static double get_average_current(u8 read_num,u8 delete_num)
* 功能：首先使用最大量程档位测试数据，根据数据的范围，再次切换档位
* 参数：read_num ：读取的数据次数
delete_num ：去掉的收尾无效数据的个数
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2025-02-16
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

double get_average_current(u8 read_num,u8 delete_num)
{
	// static u8 test_level_select_falg = 0 ;  // 用于判断最大量程的采集数据 
	double read_current = 0.0 ;              // 用于保存读取到的电流值
	if(test_level_select_falg == 0)          // 第一次切换
	{
		test_level_select_falg = 6 ; // 最大量程测试一次后，不用再切换
		relay_ctrl(CH1, H) ;  // 默认使用最大量程来测试 （PA0 0.02 200）
		systick_delay_ms(20); // 延时，保证继电器切换稳定
		//read_current = get_adc0_average_current(read_num,delete_num) ; // 用最大量程读取一次电流数据
	}
	if (test_level_select_falg == 6) // 表示继续使用最大量程来测试电流数据  0
	{
		usart1_printf("========================1\r\n");
		read_current = auto_get_adc_average_current(read_num,delete_num,PA0_0,SHUNT_RESISTOR_0_02R) ;
	}
	else if(test_level_select_falg == 1 )
	{
		read_current = auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_5R) ;
	}
	else if(test_level_select_falg == 2 )
	{
		read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_5R) ;
	}
	else if(test_level_select_falg == 3 )
	{
		usart1_printf("========================2\r\n");
		read_current = auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_300R) ;
	}
	else if(test_level_select_falg == 4 )
	{
		read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_300R) ;
	}
	else if(test_level_select_falg == 5 )
	{
		read_current = auto_get_adc_average_current(read_num,delete_num,PA2_2,SHUNT_RESISTOR_300R) ;
	}
	
	if(read_current >= 0.810) // 大于0.81A, 超过最大量程
	{
		// 停止测试 (超过量程)
		return -1.0 ;
	}
	else if( (read_current >= 0.030 ) && (read_current < 0.060)  ) //  【0.030---0.060 )  0.066档位
	{
		test_level_select_falg = 1 ; // 0.066 档位标记
		usart1_printf("test_level_select_falg = 1\r\n");
		// 切换档位，使用 5R   10  PA1
		relay_ctrl(CH1, M);
		systick_delay_ms(20);// 延时，保证继电器切换稳定
		read_current = auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_5R) ;
		
		if(read_current >= 0.060 ) // 此档位可以测试 max = 0.066 A 的电流，>= 0.060 需要升档
		{
			relay_ctrl(CH1, H) ;           // 默认使用最大量程来测试 （PA0 0.02 200）
			systick_delay_ms(20);          // 延时，保证继电器切换稳定
			test_level_select_falg = 6 ;   // 升档为0.825 档位
			read_current = auto_get_adc_average_current(read_num,delete_num,PA0_0,SHUNT_RESISTOR_0_02R) ;
		}
		else if(read_current  < 0.030)     // 此档位可以测试 max = 0.066 A 的电流，< 0.030 需要降档
		{
			// 切换档位，使用 5R   10*2  PC3
			relay_ctrl(CH1, M);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 2 ;   // 降档为 0.033档位
			read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_5R) ;
		}
		return read_current ;
		

	}// 0.002216117
	else if( (read_current >= 0.0009) && (read_current < 0.030)) //  【0.0009 ----0.030)   0.033 档位
	{
		test_level_select_falg = 2 ;  //  0.033档位标记
		usart1_printf("test_level_select_falg = 2\r\n");
		// 切换档位，使用 5R   10*2  PC3
		relay_ctrl(CH1, M);
		systick_delay_ms(20);// 延时，保证继电器切换稳定
		read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_5R) ;
		if(read_current >= 0.030) // 升档 0.066档位
		{
			// 切换档位，使用 5R   10  PA1
			relay_ctrl(CH1, M);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 1 ; // 0.066 档位标记
			read_current = auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_5R) ;
		}
		else if(read_current < 0.0009)  // 降档  0.001 档位
		{
			// 切换档位，使用 300R   10*  PA1
			relay_ctrl(CH1, L);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 3 ;  // 0.001 档位标记
			read_current = auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_300R) ;
		}
		return read_current ;
	}
	else if((read_current >= 0.0000495) && (read_current < 0.0009)) // 【0.0000495 ---- 0.0009 )  0.001 档位
	{
		test_level_select_falg = 3 ;  // 0.001 档位标记
		usart1_printf("test_level_select_falg = 3\r\n");
		// 切换档位，使用 300R   10*  PA1
		relay_ctrl(CH1, L);
		systick_delay_ms(20);// 延时，保证继电器切换稳定
		read_current = auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_300R) ;
		if(read_current >= 0.0009) // 升档 0.033档位
		{
			// 切换档位，使用 5R   10*2  PC3
			relay_ctrl(CH1, M);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 2 ;  //  0.033档位
			read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_5R) ;
		}
		else if(read_current < 0.0000495 )  // 降档 0.000055 档位
		{
			// 切换档位，使用 300R   10* 2  PC3
			relay_ctrl(CH1, L);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 4 ;  // 0.000055 档位
			read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_300R) ;
		}
		return read_current ;

	}
	else if( (read_current >= 0.00000009) && (read_current < 0.0000495)) // 【0.00000009-------0.0000495)    0.000055 档位
	{
		test_level_select_falg = 4 ;  // 0.000055 档位标记
		// 切换档位，使用 300R   10* 2  PC3
		relay_ctrl(CH1, L);
		systick_delay_ms(20);// 延时，保证继电器切换稳定
		read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_300R) ;
		if(read_current >= 0.0000495)      // 升档
		{
			// 切换档位，使用 300R   10*  PA1
			relay_ctrl(CH1, L);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 3 ;  // 0.001 档位
			read_current = auto_get_adc_average_current(read_num,delete_num,PA1_1,SHUNT_RESISTOR_300R) ;
		}
		else if(read_current  < 0.00000009)  // 降档
		{
			// 切换档位，使用 300R   10*6.35  PA2
			relay_ctrl(CH1, L);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 5 ;   // 0.0000001 档位
			read_current = auto_get_adc_average_current(read_num,delete_num,PA2_2,SHUNT_RESISTOR_300R) ;
		}
		return read_current ;

	}
	else if( (read_current >= 0) && (read_current < 0.00000009)) // 【 0------0.00000009 )   0.0000001 档位
	{
		test_level_select_falg = 5 ;   // 0.0000001 档位 标记
		// 切换档位，使用 300R   10*6.35  PA2
		relay_ctrl(CH1, L);
		systick_delay_ms(20);// 延时，保证继电器切换稳定
		read_current = auto_get_adc_average_current(read_num,delete_num,PA2_2,SHUNT_RESISTOR_300R) ;
		if(read_current >= 0.00000009)  // 升档
		{
			// 切换档位，使用 300R   10* 2  PC3
			relay_ctrl(CH1, L);
			systick_delay_ms(20);// 延时，保证继电器切换稳定
			test_level_select_falg = 4 ;  // 0.000055 档位
			read_current = auto_get_adc_average_current(read_num,delete_num,PC3_13,SHUNT_RESISTOR_300R) ;
		}
		
		return read_current ;

	}
	else //  前面如果全部不用切换档位，那就还是使用最大量程测试    【0.060 -----0.810】  0.825 档位
	{
		test_level_select_falg = 6 ; // 执行到这里，表示最大量程没有切换，   0.825 档位标记
		return  read_current;
	}
}
/*******************************************************************
********************************************************************
* 名称：void display_current(void)
* 功能： 按照采集的次数， 去无效数据后，平均取电流并显示在lcd的特定位置
* 参数：
read_num   ：读取的总次数
delete_num ：去掉的无效数据的个数
* 返回值：无
* 版本：v1
* 作者：matt
* 日期：2025-02-16
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void display_current(u8 read_num,u8 delete_num)
{
	double test_current = 0.0 ; // 保存读取到的电流值（单位 A）
	test_current = get_average_current(read_num,delete_num); // 读取read_num次，去掉首位各delete_num个无效数据，取平均，获取电流
	if(test_current == -1.0) // 超过量程
	{
		usart1_printf("OVERRANGE\r\n");
		lcd_fill_color(44,72,228,96,Cyan) ;  // 清除上次显示
		display_ascii_string(44,72,12,(u8*)"OVERRANGE",Red,Cyan);         // channel 1 处显示
	}
	else // 没有超过量程
	{
		if(test_current >= 0.001 )                                        // >= 0.001 A(1mA) 以上的电流，以mA 为单位
		{
			lcd_fill_color(44,72,228,96,Cyan) ;                           // 清除上次显示
			memset((void*)string_buf,0,sizeof((const char*)string_buf));  // 清空string_buf
			sprintf((char *)string_buf,"%.05lf mA",test_current * 1000);   // 将A 转换为mA以后显示（4位小数.0001 mA）
			usart1_printf("CHANNEL 1 [ %s ]\r\n",string_buf) ;            // 串口打印电流值
			display_ascii_string(44,72,12,(u8*)string_buf,Black,Cyan);    // channel 1 处显示
		}
		else if(test_current >= 0.000001)                            // >= 0.000001 A(1UA) 以上的电流，以uA 为单位
		{
			lcd_fill_color(44,72,228,96,Cyan) ;                          // 清除上次显示
			memset((void*)string_buf,0,sizeof((const char*)string_buf)); // 清空string_buf
			sprintf((char *)string_buf,"%.03lf uA",test_current * 1000 * 1000);   // 将A 转换为uA以后显示
			usart1_printf("CHANNEL 1 [ %s ]\r\n",string_buf) ;            // 串口打印电流值
			display_ascii_string(44,72,12,(u8*)string_buf,Black,Cyan);    // channel 1 处显示
		}
		else
		{
			lcd_fill_color(44,72,228,96,Cyan) ;                          // 清除上次显示
			memset((void*)string_buf,0,sizeof((const char*)string_buf)); // 清空string_buf
			sprintf((char *)string_buf,"%.03lf nA",test_current * 1000 * 1000 * 1000);   // 将A 转换为nA以后显示
			usart1_printf("CHANNEL 1 [ %s ]\r\n",string_buf) ;            // 串口打印电流值
			display_ascii_string(44,72,12,(u8*)string_buf,Black,Cyan);    // channel 1 处显示
		}
		

	}

}



