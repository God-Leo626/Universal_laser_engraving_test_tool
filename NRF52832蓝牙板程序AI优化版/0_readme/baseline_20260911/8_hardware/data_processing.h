#ifndef __DATA_PROCESSING_H__
#define __DATA_PROCESSING_H__
#include "stdbool.h"
#include "stdint.h"

extern uint8_t  feature_raw[32];       // 用于保存在feature_set 函数中转换号的十六进制值
extern uint8_t  feature_raw_len ;      // 用于保存在feature_set 函数中转换号的十六进制的个数
extern bool feature_global_flag  ;     //  用于保存 feature_set 函数转换是否成功的标志，  真 = 转换ok



//*****************************************************************************************************************

long num_str_to_num(char *p_numchar,bool *ok_ng,uint8_t base);
bool str_extract(const char *src,                                      // 源字符串
                 const char *start,                                    // 起始标记
                 const char *end,                                      // 结束标记
                 char *dst) ;                                           // 保存找到的字符串
void feature_set(const char *hex_str);
bool  decrypt_data(uint8_t * data_string, unsigned char string_len, uint8_t * target_buf, int buf_size);

#endif

