#include "arithmetic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "usart_dma_all.h"
/*====================================================================*/

/*******************************************************************
********************************************************************
* 名称：double quadratic_fit(double *x, double *y, int n, double *a)
* 功能：二次拟合算法
* 参数：x ：要拟合的一组双精度数据集合
        y ：目标数据集合
        n：拟合的数据个数
        a：二次拟合
        系数集合
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-17
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void quadratic_fit(double *x, double *y, int n, double *a) 
{
    // sumx  保存x的和
    // sumy  保存y的和
    // sumx2 保存x的平方和
    // sumxy 保存xy的和
    double sumx = 0, sumy = 0, sumx2 = 0, sumxy = 0;
    int i = 0; // 用于循环
    // 循环求各种和
    for (i = 0; i < n; i++) 
    {
        sumx += x[i];
        sumy += y[i];
        sumx2 += x[i] * x[i];
        sumxy += x[i] * y[i];
    }
    usart1_printf("%f\r\n",sumx);
    usart1_printf("%f\r\n",sumy);
    usart1_printf("%f\r\n",sumx2);
    usart1_printf("%f\r\n",sumxy);
    double delta = sumx * sumx - n * sumx2;
    // 二次拟合系数 
    a[1] = (n * sumxy - sumx * sumy) / delta;     // 一次项系数
    a[0] = (sumy - a[1] * sumx) / n;              // 常数项
    a[2] = (sumx2 * sumy - sumx * sumxy) / delta; // 二次项系数

   
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void calculate_quadratic_coefficients(float* data_x, float* data_y, u8 size, float* a, float* b, float* c) 
{
    double sum_x  = 0.0;
    double sum_x2 = 0.0;
    double sum_xy = 0.0;
    double sum_y  = 0.0;
    double n = size;
 
    for (int i = 0; i < size; i++) 
    {   
        sum_x  += data_x[i];
        
        sum_x2 += data_x[i] * data_x[i];
        
        sum_xy += data_x[i] * data_y[i];
        
        sum_y  += data_y[i];
        
    }
    usart1_printf("[ sum_x  ]= %f\r\n",sum_x);
    usart1_printf("[ sum_x2 ]= %f\r\n",sum_x2);
    usart1_printf("[ sum_xy ]= %f\r\n",sum_xy);
    usart1_printf("[ sum_y  ]= %f\r\n",sum_y);
    
    *a = (n * sum_xy - (sum_x * sum_y)) / ((n * sum_x2 )- (sum_x * sum_x));
    *b = (sum_y - (*a * sum_x2)) / n;
    *c = (*a / 2) * ((sum_x2 / n )- (sum_x * sum_x / n));
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
double calculate_fit(double x, float a, float b, float c) 
{
    return a * x * x + b * x + c;
}

/*******************************************************************
********************************************************************
* 名称：void linear_fit(float *x, float *y, int n, float *a, float *b) 
* 功能：一次线性拟合算法
* 参数： 
   x：标准数据
   y：测试数据
   n:处理的数据个数
   a:斜率
   b:常数

* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void linear_fit(double *x, double *y, int n, float *a, float *b) 
{
    double sumX = 0, sumY = 0, sumXY = 0, sumXX = 0;
    for (int i = 0; i < n; i++) 
    {
        sumX += x[i];   //标准数据和
        sumY += y[i];   // 测试数据和
        sumXY += x[i] * y[i]; // 标准数据 * 测试数据
        sumXX += x[i] * x[i]; // 表述数据的平方和
    }
    *a = (n * sumXY - sumX * sumY) / (n * sumXX - sumX * sumX); // 斜率a
    *b = (sumY - *a * sumX) / n;                                // 常数b
}
/*******************************************************************
********************************************************************
* 名称：double calculate_linear_fit(double x, float a, float b)
* 功能：利用一次校准后的系数，计算校准后的值
* 参数：
x：测试值
a：斜率
b：常数
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
double calculate_linear_fit(double x, float a, float b)
{
    return a*x + b ;

}
/*******************************************************************
********************************************************************
* 名称：void data_swap(int16_t* a, int16_t* b)
* 功能： 交换两个数
* 参数：a : 要交换的数据指针
b: 要交换的另一个数据指针
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/
void data_swap(int16_t* a, int16_t* b)
{
	int16_t temp = 0;
	temp = *a ;
	*a = *b ;
	*b = temp ;	
}

/*******************************************************************
********************************************************************
* 名称：int16_t get_average(int16_t * data, u8 data_leng,u8 delete_num)
* 功能：将一个数组里按照从小到大的循序排序后，去掉前几个和去掉后几个数，剩余的总和求平均
* 参数：
data ：     要处理的数据指针
data_leng ：要处理的数据总个数
delete_num ： 要去掉前后的几个数据
* 返回值：平均值
* 版本：v1
* 作者：matt
* 日期：2024-06-14
* 举例：
* 备注：
* 
* 
*********************************************************************
*********************************************************************/

uint16_t get_average(int16_t * data, u8 data_leng,u8 delete_num)
{
	uint64_t  value_sum = 0 ;
	u8 i = 0 ,j = 0;
	uint16_t average = 0 ;
	for ( i = 0 ; i < data_leng ; i++)
	{
		for(j = i+1;j < data_leng;j++)
		{
			if(data[i] > data[j]) //如果前面的数大，交换两个数的位置（从小到大排列）
			{
				data_swap(&data[i],&data[j]); 
			}
		}
	}
	// for(u8 k = 0 ;k < data_leng;k++)
	// {
	// 	usart1_printf("[%d] = [%d]\r\n",k,data[k]);
	// }
	for(u8 m = delete_num; m < data_leng - delete_num; m++)  // 去掉前2个和后2个数在求和
	{
		value_sum += data[m] ;  // 去掉最大，和最小后求和
	}
	// usart1_printf("value_sum = [%d]\r\n",value_sum);
	average = (uint16_t)((value_sum) / (uint16_t)(data_leng - 2 * delete_num ) );
	// usart1_printf("average = [%d]\r\n",average);
	return average ;
}