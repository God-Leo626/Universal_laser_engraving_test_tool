#ifndef __MY_PRINTF_H__
#define __MY_PRINTF_H__

#include "string.h"
#include <stdio.h>       // vsnprintf
#include <stdarg.h>      // va_list, va_start, va_end
#include <string.h>      // strlen
#include "nrfx_uarte.h"  // uarte_send_data
//#include "nrf_52832_uarte.h"
//#include "uart_dma.h"

#include "uarte_52832_dma.h"

#define PRINT_BUF_LEN 256
//=======================================================================================
void print(const char *fmt,...);
#endif

