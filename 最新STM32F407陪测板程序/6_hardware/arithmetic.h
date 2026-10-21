#ifndef __ARITHMETIC_H__
#define __ARITHMETIC_H__
#include "stm32f4xx.h"
/*====================================================================*/

void  quadratic_fit(double *x, double *y, int n, double *a) ;
void calculate_quadratic_coefficients(float* data_x, float* data_y, u8 size, float* a, float* b, float* c) ;
double calculate_fit(double x, float a, float b, float c) ;

void linear_fit(double *x, double *y, int n, float *a, float *b) ;
double calculate_linear_fit(double x, float a, float b);


void data_swap(int16_t* a, int16_t* b);
// int16_t get_average(int16_t * data, u8 data_leng,u8 delete_num);
uint16_t get_average(int16_t * data, u8 data_leng,u8 delete_num);
#endif

