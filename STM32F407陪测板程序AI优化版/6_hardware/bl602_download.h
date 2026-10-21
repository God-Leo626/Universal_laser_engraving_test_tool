#ifndef  __BL602_DOWNLOAD_H__
#define  __BL602_DOWNLOAD_H__
#include "stm32f4xx.h"
#include  "stdbool.h"
//**************************************************************************************
#define LOAD_BOOT_HEADER_BURN_BAUDRATE    921600   // 烧录176字节boot header的波特率
#define LOAD_FIRMWARE_BURN_BAUDRATE       2500000  // 烧录用户固件时使用的波特率
#define AT_COMM_BURN_BAUDRATE             921600   // 和用户固件AT指令通信的波特率
#define RSII_LEVER                        (-60)     // RSSI 的阈值 >=

#define GET_BOOT_INFO                 0x10   // 获取602的chip id
#define LOAD_BOOT_HEADER              0x11   // 获取BootHeader(176bytes)
#define LOAD_SEGMENT_HEADER           0X17
#define LOAD_SEGMENT_DATA             0X18
#define CHECK_IMAGE                   0X19  // 检查烧录的bootload 是否正确
#define RUN_IMAGE                     0X1A   // 运行bootload 固件

#define CHIP_ERASE                    0X3C   // 用于擦除整片 Flash

#define COMPRESSED_DATA_FALSH_PROGRAM 0X3F   // 写入 Nbytes 压缩数据到 Flash 指定地址空间
#define CHIP_SHA256_READ              0X3D   // 快速校验 Flash 烧写是否正确。




//**************************************************************************************
bool bl602_in_burn_mode(void);
bool get_boot_info(char* chipid);
bool get_bl602_chipid(char* chip_id);
bool boot_load_header_176_bytes(void);
bool load_segment_header(void);
bool load_segment_data(void);
bool check_bootload_image(void);
bool run_bootload_image(void);
bool change_baud_rate(u32 baudrate);
bool chip_erase(void);
bool flash_erase(u32 start_address,u32 end_address);
bool compressed_data_flash_program(u16 one_time_write_number,u32 bin_length);
bool flash_program_check(void);
bool chip_sha256_read(u32 start_addr,u32 read_len,char* result_hash);
bool chip_sha256_compare(char* chip_sha256,char* result_hash );

bool  module_enter_factory_mode(void);
bool judge_module_enter_factory_mode(void);
bool  module_enter_water_lamp(void);
bool  module_exit_water_lamp(void);
bool  module_get_rssi(void);
bool module_get_license(void);
bool module_get_fw_ver(void);
bool module_get_mac(void);
bool module_get_model_pcba(void);
bool module_license_wirte(void);
bool module_get_mac_string(void);
bool module_license_delete(void);
bool module_license_existence_or_not(void)  ;


#endif


