#include "ble_app.h"

#include "nrf_sdh_ble.h"   // 使用NRF_BLE_SCAN_DEF包含
#include "nrf_ble_scan.h"
#include "app_error.h"
#include "nrf_sdh.h"   //  nrf_sdh_enable_request  函数使用
#include "app_timer.h"
#include "led.h"
#include "my_printf.h"
#include "data_processing.h"
//#include <cmsis_gcc.h>   // 使用__DMB()
//#include <core_cm4.h>  // 使用__DMB()
//*****************************************************************************************************************

NRF_BLE_SCAN_DEF(m_scan);           // 定义名称为m_scan的扫描器实例         


static void scan_evt_handler(scan_evt_t const * p_scan_evt) ;  // 函数声明
static void scan_start(void) ;                                 // 函数声明

static adv_evt_t  g_adv_buf[ADV_EVT_BUF_SIZE];   // 定义一个结构体数组，保存接收到的广播数据
volatile uint8_t           g_adv_wr = 0;           // 中断里写索引
 volatile uint8_t           g_adv_rd = 0;           // 主循环里读索引
bool run_flag_a = false ;                          //  是否运行开始扫描广播数据的标志位，  true = 运行， false = 不运行  
uint8_t  mac_sn_buf [22] = {0};                   // 保存解密后的数据（数据里面有mac + sn）
/*******************************************************************
********************************************************************
* 名称：
* 功能：扫描参数定义（在初始化扫描中要使用）
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
*       filter_policy = 
0x00	BLE_GAP_SCAN_FP_ACCEPT_ALL	接收所有广播包（定向包除外）
0x01	BLE_GAP_SCAN_FP_WHITELIST	仅接收白名单中设备的广播包
0x02	BLE_GAP_SCAN_FP_ALL_NOT_RESOLVED_DIRECTED	同 0x00，额外接受无法解析的定向广播
0x03	BLE_GAP_SCAN_FP_WHITELIST_NOT_RESOLVED_DIRECTED	同 0x01，额外接受无法解析的定向广播

*********************************************************************
*********************************************************************/
static ble_gap_scan_params_t const m_scan_param =
{
    .active        = 0,                               // 0 = 被动扫描, 1 = 主动扫描
    .interval      = NRF_BLE_SCAN_SCAN_INTERVAL,      // 扫描间隔      设置为：160*0.625ms = 100ms   75ms
    .window        = NRF_BLE_SCAN_SCAN_WINDOW,        // 扫描窗口      设置为：80*0.625ms = 50ms      48 * 0.625 = 30ms
    .timeout       = 0,                               // 扫描超时时间   设置为0，即扫描不超时
    .scan_phys     = BLE_GAP_PHY_1MBPS ,               // 1 Mbps PHY
    .filter_policy = BLE_GAP_SCAN_FP_ACCEPT_ALL,      // 接收除了不是发给本机的定向广播之外的所有广播数据
};
/*******************************************************************
********************************************************************
* 名称：
* 功能： 初始化扫描参数
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
static void scan_init(void) 
{
    ret_code_t          err_code;                                         // SDK 函数返回的错误码
	  
    nrf_ble_scan_init_t  init_scan;                                       // 定义扫描初始化结构体变量
    memset(&init_scan, 0, sizeof(init_scan));                             // 先清零结构体变量，再配置
    
    init_scan.connect_if_match = false;                                   // 被动扫描不连接 （匹配后是否自动连接 真：自动连接， 假：不连接）
	init_scan.p_conn_param     = NULL;                                    // 自动连接时的参数（connect_if_match=true 时有效）
    init_scan.p_scan_param     = &m_scan_param;                           // 使用初始化结构体中的扫描参数配置扫描器，这里p_scan_param指向定义的扫描参数
    init_scan.conn_cfg_tag     = APP_BLE_CONN_CFG_TAG;                    // conn_cfg_tag设置为1，连接配置标签
    
    err_code = nrf_ble_scan_init(&m_scan, &init_scan, scan_evt_handler);  // 初始化扫描器，并注册事件句柄（用于通知app= 扫描事件处理函数）不连接，这一行在该场景下实际无效
    APP_ERROR_CHECK(err_code);
}


/*******************************************************************
********************************************************************
* 名称：
* 功能：应用程序扫描事件回调函数
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* #include "app_util_platform.h" // CRITICAL_REGION
 CRITICAL_REGION_ENTER();
    ret_code_t err = nrf_ringbuf_put(&m_ble_rx_rb, p_data, len);
    CRITICAL_REGION_EXIT();
* 
    type	ble_gap_adv_report_type_t	   广播报告类型（bitfield）见下方
    peer_addr	ble_gap_addr_t	           广播设备的蓝牙地址（含地址类型和地址值）
    direct_addr	ble_gap_addr_t	           定向广播的目标地址（仅 type.directed=1 时有效）
    primary_phy	uint8_t	                   主广播信道 PHY（BLE_GAP_PHY_1MBPS 等）
    secondary_phy	uint8_t	               辅广播信道 PHY（无辅信道时为 BLE_GAP_PHY_NOT_SET）
    tx_power	int8_t	                   广播设备发射功率（dBm），无效时为 BLE_GAP_POWER_LEVEL_INVALID
    rssi	int8_t	                       接收信号强度（dBm）
    ch_index	uint8_t	                   收到广播包的信道号（37、38、39）
    set_id	uint8_t	                       广播集 ID（扩展广播），BLE_GAP_ADV_REPORT_SET_ID_NOT_AVAILABLE 表示不支持
    data_id	uint16_t:12	                   广播数据 ID（10位），用于去重
    data	ble_data_t	                   广播数据内容（核心字段），结构：{.p_data=指针, .len=长度}
    aux_pointer	ble_gap_aux_pointer_t	   扩展广播下一包偏移（仅 status=INCOMPLETE 时有效）

127 = BLE_GAP_POWER_LEVEL_INVALID，意思是广播者没有在广播包里附带 Tx Power 字段
*********************************************************************
*********************************************************************/
static void scan_evt_handler(scan_evt_t const * p_scan_evt)
{
    switch (p_scan_evt->scan_evt_id)   //判断事件类型
    {
        
        case NRF_BLE_SCAN_EVT_NOT_FOUND:   //因为不过滤扫描信息，即不设置过滤条件，所以只需关注NRF_BLE_SCAN_EVT_NOT_FOUND事件
        { 
            ble_gap_evt_adv_report_t const * p_adv = p_scan_evt->params.p_not_found; //定义广播报告结构体指针p_adv并指向扫描事件结构体中的p_not_found
            
//            if ((feature_global_flag == true) && (feature_raw_len > 0))
//            {
//                if ((p_adv->data.len < feature_raw_len) ||   (memcmp(p_adv->data.p_data, feature_raw, feature_raw_len) != 0))   
//                {
//                    break;   // 不匹配直接丢弃，不进缓冲
//                }
//            }
            
            
            uint8_t next = (g_adv_wr + 1) % ADV_EVT_BUF_SIZE;  // 将下一个写入位置指向结构体的下一个位置
            if (next == g_adv_rd)                              // 如果更新后的下一个写入位置和读入数据的位置重叠，说明环形缓冲器结构体数组已经满了
            {
                return;                                        // 缓冲满，丢弃
            }

            adv_evt_t * p = &g_adv_buf[g_adv_wr];   //  定义一个结构体指针，指向结构体数组
            p->peer_addr = p_adv->peer_addr;        //  广播中的设备地址赋值给结构体数组中的 peer_addr
            p->rssi      = p_adv->rssi;             //  广播中的rssi值赋值给结构体数组中的  p->rssi 
            p->ch_index  = p_adv->ch_index;         //  广播中的信道号，赋值给结构体数组中的   p->ch_index 
            p->data_len  = p_adv->data.len;         //  广播中的数据长度赋值给结构体数组中的  p->data_len
            p->tx_power  = p_adv->tx_power;         //  广播中的发送者的发送功率赋值给结构数组中的
            memset(p->data,'\0',31);                // 清理结构体变量中保存数据的数组空间  p->tx_power
            memcpy(p->data, p_adv->data.p_data, p_adv->data.len); // 广播中的数据内容赋值给结构体数组中的  p->data
            
            //__DMB()（Data Memory Barrier）是 Cortex-M4 的内置函数，
            // 包含在 <cmsis_gcc.h> 或 <core_cm4.h> 中。它保证在 __DMB() 
            // 之前的所有内存写入完成后，才执行后面的 g_adv_wr = next，
            // 防止 CPU 乱序执行或写缓冲导致主循环看到"写指针已推进但数据未写完"的中间状态。
                                          //   
            __schedule_barrier();   // ← ARMCC 内置内存屏障，等价于 __DMB() ,确保所有写入完成后再更新写指针
            g_adv_wr = next;                         // 更新写指针

            break;
        } 		 
        case NRF_BLE_SCAN_EVT_SCAN_TIMEOUT:           //扫描超时事件
        {
            scan_start();                             // 从启扫描
            break;
        } 
        default:
            break;
    }
}
/*******************************************************************
********************************************************************
* 名称：
* 功能：循环打印接收到的广播数据
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
void main_loop_process(void) 
{
    
    if(g_adv_rd == g_adv_wr)  // 读写指针相等的时候表示环形缓冲中为空
    {
        return;
    }
   
    adv_evt_t * p = &g_adv_buf[g_adv_rd]; // 将扫描到的数据赋值给一个结构体变量

    if (p->data_len > 31)  // 接收广播的数据长度超过31时
    {
        g_adv_rd = (g_adv_rd + 1) % ADV_EVT_BUF_SIZE;
        return;
    }

    // 扫描到的数据长度 >= HEX字符数组的长度   且  特征字符数组的长度 >0 且  特征十六进制真的设置ok 
    if( (p->data_len >= feature_raw_len ) &&  (feature_raw_len > 0) && (feature_global_flag == true) )
    {
        if (memcmp(p->data, feature_raw, feature_raw_len) == 0)   // 将 p->data中的数据和 feature_raw中的数据对比feature_raw_len个，数据一致
        {
         
            run_flag_a = true ;     // 运行特征值筛选的标志
            run_flag_b = false ;   //  自定义阈值方式时， 将默                                                                               认允许运行的标志置假， 互斥，保障自定义设置后，一定按照自定义的阈值过滤广播
        }
    }
    // run_flag_a= 自定义方式 可以执行  run_flag_b = 默认方式也可以执行
    if( run_flag_a  || run_flag_b ) // run_flag_a = true时，判断扫描接受的广播包中的数据是否包含特征值（只接收符合特征值的广播）
    {
       
        run_flag_a = false ;  // 每次进来后将run_flag_a 标志置假，所以循环中只要能进来，一定是满足run_flag_a = true 要求
        if( ( p->rssi >= rssi_buf ) && (rssi_change_result == true)) // 扫描到的rssi 要大于 等于 rssi设置阈值(rssi_change_result: 表示rssi的值真正设置ok)
        {                                                                                                    
            print("[RSSI=%d] ---> [CH=%d] ---> [tx_pwr=%d] ---> ", p->rssi, p->ch_index,p->tx_power);  // 串口输出
            
            //  默认不显示原始数据
            if(mask_raw_data_global_flag == true )  //  原始数据打印与否的开关 ， 真 就打印（通过串口命令才能设置为真）
            {
                if (byte_order_flag == false) // mac  默认顺序输出
                {
                    for (int i = 0; i < 6; i++)   // 将MAC的6个字节打印成hex数据
                    {
                        if(i == 0 )
                        {
                            print("[MAC=0X");
                        }
                        print("%02X ", p->peer_addr.addr[i]);
                        if(i == 5)
                        {
                            print("] ");
                        }
                    }
                }
                else  // mac 逆序输出
                {
                    for (int i = 5; i >= 0; i--)   // 将MAC的6个字节打印成hex数据
                    {
                        if(i == 5 )
                        {
                            print("[MAC=0X");
                        }                                    
                        print("%02X ", p->peer_addr.addr[i]);
                        if(i == 0)
                        {
                            print("] ");
                        }
                    }
                
                }
                
                // 将广播中的数据打印成hex数据
                for (int i = 0; i < p->data_len; i++) // 将广播中的数据打印成hex数据
                {
                    if( i == 0 )
                    {
                        print("---> [DATA=0X");
                    }
                    print("%02X ", p->data[i]);
                    if(i == p->data_len - 1)
                    {
                        print("]");
                    }
                }    
                print("\r\n");
            }
            // ==============【 处理ble接收到的数据解析方式，  按照异或解析还原数据， 或者按照明文解析数据】
            if(decoding_set_global_flag == false) // 默认时false，按照xor异或算法解析数据
            {
                //  解析广播中的加密数据并打印成十六进制
               if( decrypt_data(p->data, p->data_len,mac_sn_buf,sizeof(mac_sn_buf)) == true)
               {
    //                print("MAC_SN = ") ;
                    if( mac_sn_buf[0] == 0x04 ) // 举例数据：[04] 06 DC5475A87ACC 0507 2302510000029800303838  （04=mac）
                    {
                        if(mac_sn_buf[1] == 0x06) // mac 长度是6个字节
                        {
                            print("[DECRYPT_MAC]=");
                            for(unsigned char i = 2 ; i < mac_sn_buf[1] + 2; i++) //  从解析出来的第0,1,2个字节开始就是mac数据的开始
                            {
                                print("%02X",mac_sn_buf[i]);
                            }  
                        }
                        
    //                    print("\r\n");
                    }
                    if(mac_sn_buf[8] == 0x05) // 举例数据：0406 DC5475A87ACC [05]07 2302510000029800303838  （05=sn）
                    { 
                        if(mac_sn_buf[9] == 0x07)  //  sn的长度是 0x07 7个字节
                        {
                            print(" ---> [DECRYPT_SN]=");
                            for(unsigned char i = 10 ; i < mac_sn_buf[9] + 10 ; i++)  // 从解析出来的0,1，....10个字符开始就是 sn数据
                            {
                                print("%02X",mac_sn_buf[i]);
                            }       
                        }
                        
                        print("\r\n");
                    } 
               }
//               else
//               {
//                    print("[ advertising_data < 31 ]\r\n");   //  当没有满足要求长度= 31字节数据的时候，打印提示
//               }
            
            }
            // 明文数据结构(一共 31 个字节 )
            // AD Structure1    Len  AD Type   Comp ID    Pro ID   ChkSum  Encrypt Idx   Payload
            // 02  01  xx       1B     FF       CA  0A     FE        1字节      1字节       21字节
            //                                                     [8 ----23]    [24 -------30]
            //                                                      chip id           sn   
            //                                                      16个字节         7个字节
            else  // decoding_set_global_flag = true 的时候，表示按照明文方式解析数据
            {
                if (p->data_len == 31)  // 收到的数据长度是 31字节
                {
                    print("[DECRYPT_MAC]=");
                    // 从p->data 的【8】 开始 就是唯一码数据
                    // 31 数据的总长度， -7 = sn占用长度， -8=特征值占用长度 +8= i从8开始
                    for(unsigned char i = 8 ; i < 31 - 7 - 8 + 8; i++) 
                    {
                        print("%02X",p->data[i]);
                    }
                   
                    print(" ---> [DECRYPT_SN]=");
                    //  31个字节里， 从24开始  到 31结束，就是sn数据
                    for(unsigned char i = 24 ; i < 24 + 7; i++)  
                    {
                        print("%02X",p->data[i]);
                    } 
                    print("\r\n");
                }
//                else
//                {
//                    print("[ advertising_data is not 31 ]\r\n");
//                } 
            }  
        }
    }
//    else
//    {
//        static uint32_t skip_count = 0;
//        if(++skip_count % 100 == 0)
//        print("[skip=%lu]\r\n", skip_count);
//    }
    // __schedule_barrier() 是 ARMCC 的内建函数，无需任何额外头文件，直接可用
    // 它的作用是阻止编译器重排内存访问顺序，确保屏障前所有写入操作完成后才执行屏障后的代码。
    __schedule_barrier();
    g_adv_rd = (g_adv_rd + 1) % ADV_EVT_BUF_SIZE;  //  读地址移动到下一个结构体数组
    
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
//void main_loop_process(void)
//{
//    static char   out_buf[256];           // 批量输出缓冲区，一次 sprintf 填满
//    static adv_evt_t saved_adv[10];       // 拷贝保存的匹配广告（完整结构体）
//    static int8_t saved_count = 0;        // 本轮积累的匹配广告条数

//    // ===== 第一阶段：快扫环形缓冲区，整条拷出匹配广告 =====
//    while (g_adv_rd != g_adv_wr)          // 缓冲区不为空就继续
//    {
//        adv_evt_t *p = &g_adv_buf[g_adv_rd]; // 取当前槽位

//        if (p->data_len > 31)             // 数据异常（超过 BLE 最大长度）
//        {
//            g_adv_rd = (g_adv_rd + 1) % ADV_EVT_BUF_SIZE; // 跳过本条
//            continue;                     // 继续下一条
//        }

//        // 特征值匹配判定
//        bool matched = false;             // 本条是否匹配用户设置的特征
//        if ((p->data_len >= feature_raw_len)       // 长度够做比较
//            && (feature_raw_len > 0)               // 用户设了特征
//            && (feature_global_flag == true))       // 特征转换成功
//        {
//            if (memcmp(p->data, feature_raw, feature_raw_len) == 0) // 逐字节比
//            {
//                matched = true;            // 命中
//                run_flag_a = true;         // 开闸：本条允许输出
//            }
//        }

//        if (run_flag_a || run_flag_b)     // 满足输出条件
//        {
//            if (matched) run_flag_a = false; // 用完就清，避免泄漏到后面

//            if ((p->rssi >= rssi_buf)               // RSSI 达标
//                && (rssi_change_result == true)      // RSSI 设置成功
//                && saved_count < 10)                 // 最多存 10 条
//            {
//                saved_adv[saved_count++] = *p;       // 整条拷贝，脱离环形缓冲区
//            }
//        }

//        __schedule_barrier();             // 确保本条所有读取完成后
//        g_adv_rd = (g_adv_rd + 1) % ADV_EVT_BUF_SIZE; // 才推进读指针，允许 ISR 覆写
//    }

//    // ===== 第二阶段：对本地拷贝批量输出，与 ISR 无竞态 =====
//    for (int n = 0; n < saved_count; n++) // 轮完本轮所有匹配广告
//    {
//        adv_evt_t *p = &saved_adv[n];     // 取本地拷贝，不再碰环形缓冲区
//        int pos = 0;                      // out_buf 写入游标

//        // 行 1：RSSI / 信道 / 发射功率
//        pos += sprintf(out_buf + pos,     // 拼入 out_buf
//                       "[RSSI=%d] ---> [CH=%d] ---> [tx_pwr=%d] ---> ",
//                       p->rssi, p->ch_index, p->tx_power);

//        // ===== 原始数据打印开关（原有逻辑）=====
//        if (mask_raw_data_global_flag == true) // 命令开关打开才输出原始数据
//        {
//            if (byte_order_flag == false)  // MAC 默认正序输出
//            {
//                pos += sprintf(out_buf + pos, "[MAC=0X");  // MAC 前缀
//                for (int i = 0; i < 6; i++) // 6 字节 MAC 逐一转 hex
//                {
//                    pos += sprintf(out_buf + pos, "%02X ", p->peer_addr.addr[i]);
//                }
//                pos += sprintf(out_buf + pos, "] "); // MAC 后缀
//            }
//            else                           // MAC 逆序输出
//            {
//                pos += sprintf(out_buf + pos, "[MAC=0X");  // MAC 前缀
//                for (int i = 5; i >= 0; i--) // 从第 5 字节往回逐一转 hex
//                {
//                    pos += sprintf(out_buf + pos, "%02X ", p->peer_addr.addr[i]);
//                }
//                pos += sprintf(out_buf + pos, "] "); // MAC 后缀
//            }

//            // 广播原始数据整体转 hex
//            pos += sprintf(out_buf + pos, "---> [DATA=0X"); // DATA 前缀
//            for (int i = 0; i < p->data_len; i++) // 按实际长度逐字节转 hex
//            {
//                pos += sprintf(out_buf + pos, "%02X ", p->data[i]);
//            }
//            pos += sprintf(out_buf + pos, "]\r\n");  // DATA 后缀 + 换行
//        }

//        // 明文模式分支
//        if (decoding_set_global_flag == true) // 用户选了明文解码
//        {
//            if (p->data_len == 31)        // 必须是 31 字节的广告
//            {
//                pos += sprintf(out_buf + pos, "[DECRYPT_MAC]="); // MAC 前缀
//                for (unsigned char i = 8; i < 24; i++)  // 第 8—23 字节为 MAC
//                    pos += sprintf(out_buf + pos, "%02X", p->data[i]);

//                pos += sprintf(out_buf + pos, " ---> [DECRYPT_SN]="); // SN 前缀
//                for (unsigned char i = 24; i < 31; i++) // 第 24—30 字节为 SN
//                    pos += sprintf(out_buf + pos, "%02X", p->data[i]);
//            }
//        }
//        // XOR 解密分支
//        else                              // 用户选了密文解码
//        {
//            if (decrypt_data(p->data, p->data_len,  // XOR 解密
//                             mac_sn_buf, sizeof(mac_sn_buf)))
//            {
//                if (mac_sn_buf[0] == 0x04             // MAC 标记位
//                    && mac_sn_buf[1] == 0x06)          // MAC 长度 6
//                {
//                    pos += sprintf(out_buf + pos, "[DECRYPT_MAC]=");
//                    for (unsigned char i = 2; i < mac_sn_buf[1] + 2; i++) // 解密后 MAC
//                        pos += sprintf(out_buf + pos, "%02X", mac_sn_buf[i]);
//                }
//                if (mac_sn_buf[8] == 0x05             // SN 标记位
//                    && mac_sn_buf[9] == 0x07)          // SN 长度 7
//                {
//                    pos += sprintf(out_buf + pos, " ---> [DECRYPT_SN]=");
//                    for (unsigned char i = 10; i < mac_sn_buf[9] + 10; i++) // 解密后 SN
//                        pos += sprintf(out_buf + pos, "%02X", mac_sn_buf[i]);
//                }
//            }
//        }

//        pos += sprintf(out_buf + pos, "\r\n");  // 行尾回车换行

//        uart_dma_send((uint8_t *)out_buf, pos); // 整条一次 DMA 搬走
//        while (uart_dma_is_tx_busy()) {}        // 等 DMA 发完
//    }

//    saved_count = 0;                       // 重置，准备下一轮
//}




/*******************************************************************
********************************************************************
* 名称：
* 功能： BLE协议栈 初始化
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
static void ble_stack_init(void)
{
    ret_code_t err_code;                  //接收函数返回的错误码
    err_code = nrf_sdh_enable_request();  //请求使能SoftDevice，该函数中会根据sdk_config.h文件中低频时钟的设置来配置低频时钟
    APP_ERROR_CHECK(err_code);
    
    // 用 sdk_config.h 中配好的值配置 BLE 参数
    uint32_t ram_start = 0;                                                     //定义保存应用程序RAM起始地址的变量
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);   //使用sdk_config.h文件的默认参数配置协议栈，获取应用程序RAM起始地址，保存到变量ram_start
    APP_ERROR_CHECK(err_code);

    // 启用 BLE 功能（使扫描可用
    err_code = nrf_sdh_ble_enable(&ram_start);  //使能BLE协议栈
    APP_ERROR_CHECK(err_code);
}

/*******************************************************************
********************************************************************
* 名称：
* 功能： 初始化APP定时器模块
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
//static void timers_init(void)
//{
//    ret_code_t err_code = app_timer_init();  //初始化APP定时器模块  
//    APP_ERROR_CHECK(err_code);  //检查返回值

//    //加入创建用户定时任务的代码，创建用户定时任务。 
//}

/*******************************************************************
********************************************************************
* 名称：
* 功能： 启动扫描
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
static void scan_start(void)
{
    ret_code_t ret;
    
    ret = nrf_ble_scan_start(&m_scan);  //启动扫描器m_scan
    APP_ERROR_CHECK(ret);
   
}

/*******************************************************************
********************************************************************
* 名称：
* 功能： ble被动扫描初始化
* 参数：
* 返回值：
* 版本：v1
* 作者：matt
* 日期：2026-06-06
* 举例：
* 备注：
* 
*  1 ：协议栈初始化
   2 ：扫描参数初始化
   3 ：扫描使能
*********************************************************************
*********************************************************************/
void  ble_passive_scanning_init (void)
{
    ble_stack_init();  // 1 ：协议栈初始化
    scan_init();       // 2 ：扫描参数初始化
    scan_start();      // 3 ：扫描使能

}



