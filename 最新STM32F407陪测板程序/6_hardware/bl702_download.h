#ifndef  __BL702_DOWNLOAD_H__
#define  __BL702_DOWNLOAD_H__
#include "stdbool.h"
#include "stm32f4xx.h"
//*********************************************************************************************************************
#define BL702_LOAD_BOOT_HEADER_BURN_BAUDRATE           921600   // 烧录176字节boot header的波特率
#define BL702_LOAD_FIRMWARE_BURN_BAUDRATE              2500000  // 烧录用户固件时使用的波特率
#define BL702_AT_COMM_BURN_BAUDRATE                    921600   // 和用户固件AT指令通信的波特率
#define BL702_MODULE_POWERON_COMM_BAUDRATE             2000000  // 702模块上电往串口丢数据时的波特率
#define BL702_RSII_LEVER                              (-60)     // RSSI 的阈值 >=

#define BL702_GET_BOOT_INFO                 0x10   // 获取602的chip id
#define BL702_LOAD_BOOT_HEADER              0x11   // 获取BootHeader(176bytes)
#define BL702_LOAD_SEGMENT_HEADER           0X17
#define BL702_LOAD_SEGMENT_DATA             0X18
#define BL702_CHECK_IMAGE                   0X19  // 检查烧录的bootload 是否正确
#define BL702_RUN_IMAGE                     0X1A   // 运行bootload 固件

#define BL702_CHIP_ERASE                    0X3C   // 用于擦除整片 Flash

#define BL702_FALSH_PROGRAM                 0X31  // 写入 Nbytes 压缩数据到 Flash 指定地址空间
#define BL702_CHIP_SHA256_READ              0X3D   // 快速校验 Flash 烧写是否正确。



//*********************************************************************************************************************
bool bl702_in_burn_mode(void);
bool bl702_get_boot_info(void);
bool get_bl702_chipid(void);
bool bl702_boot_load_header_176_bytes(void);
bool bl702_load_segment_header(void);
bool bl702_load_segment_data(void);
bool bl702_check_bootload_image(void);
bool bl702_run_bootload_image(void);
bool bl702_change_baud_rate(u32 baudrate);

bool  bl702_chip_erase(void);
bool  bl702_flash_program(u16 one_time_write_number,u32 bin_length);
bool  bl702_flash_program_check(void);
bool  bl702_chip_sha256_read(u32 start_addr,u32 read_len,char* result_hash);
bool  bl702_chip_sha256_compare(char* chip_sha256,char* result_hash );
bool  bl702_judge_module_activate_factorymode(void);
bool bl702_enter_factorymode(void);
bool bl702_enter_waterfall_light(void);
bool bl702_get_rssi_value(signed char rssi_value);


bool bl702_get_mac(void);
bool bl702_read_fw_version(void);
bool bl702_authorization(void);


#endif




