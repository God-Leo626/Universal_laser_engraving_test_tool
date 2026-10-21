#ifndef __ESP8266_H__
#define __ESP8266_H__
#include "stm32f4xx.h"
#include "io_bit.h"
#include "stdbool.h"
//**************************************************************************************
#define WIFI_RESET         PBout(12)
#define WIFI_PWR_CTRL      PAout(0)   // PA0
//============wifi sta 模式要连接的server ip 和端口============================
extern const char* portnum ;		// 端口号 
extern const char* ip_address  ;    // IP地址


//==============esp8266要连接的wifi的 ssid 和 密码========================================
extern const char* wifista_ssid;			        // 路由器SSID号
extern const char* wifista_encryption ;	            // wpa/wpa2 aes加密方式
extern const char* wifista_password ;             	// 连接密码

extern const char* get_fw_version ;                 // 查询固件
extern const char* get_down_fw ;                    // 下载固件
extern const char* get_license_meg ;                // 查询license 信息
extern const char* down_license_meg ;               // 下载license 信息
extern const char* query_auth_number ;              // bl702 查询授权数据
extern const char* report_test_result ;             // 上报测试结果
//**************************************************************************************
void esp8266_cmd_send(char* cmd_at);
void esp8266_reset_pin_init(void);
void esp8266_reset_ctrl(u16 timer);
void esp8266_power_ctrl_pin_init(void);
void esp8266_power_ctrl(bool on_off);
void esp8266_sta_coolkit_server_connect(void);
void esp8266_sta_coolkit_server_close(void);
void esp8266_reconnection_set(void);
bool esp_8266_connect_status_check(void);
void esp_8266_at_response(u8 mode);
u8*  esp_8266_check_cmd(char *str);
bool esp_8266_send_cmd(char *cmd,char *ack,u8 times,u16 waittime);
void esp_8266_quit_passthrough (void);
void esp_8266_get_wanip(u8* ipbuf);
void esp8266_sta_mode_set_init(void);
void esp8266_sta_mode_connect_wifi_config(void);
//uint32_t esp8266_sta_http_json_tx(char* http_request,char* http_mode,char* url,const char* key, int data);
//uint32_t esp8266_sta_http_json_tx(char* http_request,char* http_mode,char* url);
uint32_t esp8266_sta_http_json_tx(char* http_request,char* http_mode,char* url,bool need_path,u32 burn_start,u32 burn_end);
uint32_t esp8266_sta_http_json_down_license_tx(char* http_request,char* http_mode,char* url,char * chip_id);
uint32_t esp8266_sta_http_json_report_result_tx(char* http_request,char* http_mode,char* url,char * device_mac,char* device_id,bool result);

bool cp601_into_burn_mode(void);

#endif

