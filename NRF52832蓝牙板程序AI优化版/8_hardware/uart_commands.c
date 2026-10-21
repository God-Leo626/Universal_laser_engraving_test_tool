/** 串口命令层：完整行校验 -> 参数校验 -> 提交配置 -> 原格式应答。 */
#include "uart_commands.h"
#include "app_config.h"
#include "data_processing.h"
#include "my_printf.h"
#include "uarte_52832_dma.h"
#include <string.h>

static char m_line[UART_COMMAND_MAX_LENGTH + 1u];
static size_t m_line_length;
static bool m_discard_line;

static void execute_command(const char *line)
{
    if (g_app_config.scanning_started) return;

    if (strncmp(line, "rssi_set=", 9) == 0) {
        bool valid;
        long value = num_str_to_num(line + 9, &valid, 10);
        if (!valid || value < RSSI_MIN_VALUE || value > RSSI_MAX_VALUE) {
            print("< rssi is not a plain numeric value, or rssi set error < -105 <= rssi <= 0 >\r\n");
            return;
        }
        g_app_config.rssi_threshold = (int8_t)value;
        g_app_config.rssi_configured = true;
        print("[ rssi = %ld , rssi_set_ok ]\r\n", value);
    } else if (strncmp(line, "feature_value_set=", 18) == 0) {
        if (!feature_set(line + 18)) {
            print("< feature must contain 2..62 hexadecimal characters, in pairs >\r\n");
            return;
        }
        print("[ feature_value = 0x");
        for (size_t i = 0; i < feature_raw_len; ++i) print("%02x", feature_raw[i]);
        print(" ]\r\n");
    } else if (strcmp(line, "start_scanning") == 0) {
        g_app_config.default_mode = false;
        g_app_config.scanning_started = true;
        print("[ start_scanning_ok ]\r\n");
    } else if (strcmp(line, "byte_order_set") == 0) {
        g_app_config.reverse_address = true;
        print("[ byte_order_set_ok ]\r\n");
    } else if (strcmp(line, "use_default_settings") == 0) {
        if (!g_app_config.rssi_configured) g_app_config.rssi_threshold = RSSI_DEFAULT_VALUE;
        g_app_config.rssi_configured = true;
        g_app_config.default_mode = true;
        g_app_config.show_raw = true;
        g_app_config.scanning_started = true;
        print("[ default_settings_ok ]\r\n");
    } else if (strcmp(line, "mask_raw_data") == 0) {
        /* 命令名称为历史协议：实际含义是打开原始数据显示。 */
        g_app_config.show_raw = true;
        print("[ mask_raw_data_ok ]\r\n");
    } else if (strcmp(line, "decoding_setting") == 0) {
        g_app_config.plaintext = !g_app_config.plaintext;
        print(g_app_config.plaintext ? "[ plaintext_set_ok ]\r\n" : "[ cipnertext_set_ok ]\r\n");
    } else {
        print("< command is error >\r\n");
    }
}

void uart_commands_discard_partial(void)
{
    m_line_length = 0;
    m_discard_line = true;
}

void uart_commands_feed(const uint8_t *data, size_t length)
{
    if (data == NULL || g_app_config.scanning_started) return;
    for (size_t i = 0; i < length && !g_app_config.scanning_started; ++i) {
        uint8_t ch = data[i];
        if (m_discard_line) {
            if (ch == '\n') m_discard_line = false;
            continue;
        }
        if (ch == '\n') {
            if (m_line_length > 0 && m_line[m_line_length - 1u] == '\r') {
                m_line[m_line_length - 1u] = '\0';
                execute_command(m_line);
            } else {
                print("< command is error >\r\n");
            }
            m_line_length = 0;
        } else if (ch == 0 || ch > 0x7f || m_line_length == UART_COMMAND_MAX_LENGTH) {
            uart_commands_discard_partial();
            print("< possible error >\r\n");
        } else {
            m_line[m_line_length++] = (char)ch;
        }
    }
}

void uart_data_processing(void)
{
    uint8_t received[UART_RX_BUF_SIZE];
    size_t length;
    bool damaged;
    if (!uart_dma_read(received, sizeof(received), &length, &damaged)) return;
    if (g_app_config.scanning_started) return;
    if (damaged) {
        uart_commands_discard_partial();
        print("< possible error >\r\n");
        return;
    }
    uart_commands_feed(received, length);
}
