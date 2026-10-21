/*
 * 应用任务：USART1 接收上位机命令，USART2 配置/接收 NRF52832。
 * 所有 LCD、Flash 和 NRF 配置均由 USART2 任务执行，避免跨任务操作外设。
 * 串口中断只收字节；本文件按协议组帧，不把一次 DMA 空闲当作完整报文。
 */
#include "my_task.h"
#include "app_protocol.h"
#include "usart_dma_all.h"
#include "dma.h"
#include "usart_ring_buff.h"
#include "variable_global.h"
#include "button.h"
#include "time3.h"
#include "lcd.h"
#include "w25q32.h"
#include "data_processing.h"
#include <stdio.h>
#include <string.h>

#define CONFIG_QUEUE_DEPTH 8U
#define NRF_LINE_CAPACITY 1025U
#define FLASH_RSSI_ADDRESS 0x00000000U
#define FLASH_FEATURE_ADDRESS 0x00000009U
#define FLASH_PROJECT_ADDRESS 0x00000028U
#define FLASH_ID_LENGTH_ADDRESS 0x00000046U
#define FLASH_OLD_COMMAND_ADDRESS 0x00000049U
#define ACK_RSSI 0x01U
#define ACK_FEATURE 0x02U

TaskHandle_t StartTask_Handler = NULL;
TaskHandle_t Usart1_Receive_Print_Task_Handler = NULL;
TaskHandle_t Usart2_Receive_Print_Task_Handler = NULL;
volatile bool usart2_cmd_set_flag = true;

typedef struct {
    uint8_t length;
    uint8_t data[APP_PC_TEXT_MAX + 1U];
} ConfigRequest;

/* 只有以下三个事务字段跨任务共享；发布/读取时使用短临界区。 */
static volatile bool pc_waiting = false;
static volatile bool pc_ack_received = false;
static volatile uint8_t pc_expected_command = 0;
static QueueHandle_t config_queue = NULL;

/* 以下状态仅由 USART2 任务访问。长度与旧命令码分别保存，不能混用。 */
static uint8_t id_length = 0;
static uint8_t old_command = 0;
static bool raw_mode = false;
static unsigned int nrf_ack_flags = 0;
static char nrf_line[NRF_LINE_CAPACITY];
static size_t nrf_line_used = 0;
static bool nrf_discard_line = false;
static unsigned int nrf_last_dropped = 0;
static AppAdvertisement candidate;
static unsigned int candidate_count = 0;
static bool candidate_conflict = false;
static AppAdvertisement reporting;
static uint8_t report_packet[APP_PACKET_MAX];
static size_t report_length = 0;
static uint32_t last_retry_slot = 0;
static unsigned int progress_x = 3;

static const uint8_t rssi_ok[] = {0x7f, 0x07, 0x81, 0x80, 0, 0, 0x06};
static const uint8_t feature_ok[] = {0x7f, 0x07, 0x82, 0x80, 0, 0, 0x05};

static void process_nrf_input(void);

/* 创建顺序不变；失败不能带着缺失的关键任务继续运行。 */
void start_task(void *pvParameters)
{
    BaseType_t first, second;
    (void)pvParameters;
    taskENTER_CRITICAL();
    config_queue = xQueueCreate(CONFIG_QUEUE_DEPTH, sizeof(ConfigRequest));
    if (config_queue == NULL) board_fatal_error("config queue allocation");
    first = xTaskCreate(usart1_receive_print_task, "usart1_receive", USART1_RECEIVE_PRINT_STK_SIZE,
                        NULL, USART1_RECEIVE_PRINT_TASK_PRIO, &Usart1_Receive_Print_Task_Handler);
    second = xTaskCreate(usart2_receive_print_task, "usart2_receive", USART2_RECEIVE_PRINT_STK_SIZE,
                         NULL, USART2_RECEIVE_PRINT_TASK_PRIO, &Usart2_Receive_Print_Task_Handler);
    if (first != pdPASS || second != pdPASS) board_fatal_error("application task allocation");
    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
}

/* PC 确认立即交付；配置命令拷贝到队列，握手等待期间不会被新参数覆盖。 */
static void dispatch_pc_frame(const uint8_t *frame, size_t length)
{
    ConfigRequest request;
    bool acknowledgement;
    taskENTER_CRITICAL();
    acknowledgement = pc_waiting &&
        ((length == 9U && memcmp(frame, "tesk_ok\r\n", 9U) == 0) ||
         app_pc_ack_matches(frame, length, pc_expected_command));
    if (acknowledgement) pc_ack_received = true;
    taskEXIT_CRITICAL();
    if (acknowledgement || (length == 7U && frame[0] == 0x7f && frame[3] == 0x80)) return;
    memset(&request, 0, sizeof(request));
    request.length = (uint8_t)length;
    memcpy(request.data, frame, length);
    if (xQueueSend(config_queue, &request, 0) != pdPASS) usart1_printf("command queue full\r\n");
}

void usart1_receive_print_task(void *pvParameters)
{
    AppPcStream stream = {{0}, 0, false};
    uint8_t bytes[128];
    unsigned int read_count, i, dropped = 0, now_dropped;
    size_t frame_length;
    TickType_t last_receive = 0;
    (void)pvParameters;
    for (;;) {
        USART1_REVEIVE_OK = false;
        now_dropped = get_ringbuffer_dropped_for_usart1(&usart1_receive_buffer);
        if (now_dropped != dropped) {
            /* 入队丢字节后，丢弃当前不完整帧，绝不拼接两侧数据。 */
            dropped = now_dropped;
            memset(&stream, 0, sizeof(stream));
            while (read_ringbuffer_count_for_usart1(&usart1_receive_buffer, (char *)bytes, sizeof(bytes)) != 0U) {}
        }
        if (stream.used != 0U && (xTaskGetTickCount() - last_receive) > pdMS_TO_TICKS(500))
            memset(&stream, 0, sizeof(stream));
        /* 每轮限制为一个环形缓冲容量，保证异常连续输入也会让出 CPU。 */
        for (i = 0; i < USART1_RING_BUFFER_LEN / sizeof(bytes); ++i) {
            unsigned int offset;
            read_count = read_ringbuffer_count_for_usart1(&usart1_receive_buffer, (char *)bytes, sizeof(bytes));
            if (read_count == 0U) break;
            now_dropped = get_ringbuffer_dropped_for_usart1(&usart1_receive_buffer);
            if (now_dropped != dropped) { memset(&stream, 0, sizeof(stream)); break; }
            last_receive = xTaskGetTickCount();
            for (offset = 0; offset < read_count; ++offset) {
                frame_length = app_pc_feed(&stream, bytes[offset]);
                if (frame_length != 0U) dispatch_pc_frame(stream.data, frame_length);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void show_setting(unsigned int y, const char *text)
{
    lcd_fill_color(85, y, 317, y + 16U, Black);
    display_ascii_string(85, y, 8, (u8 *)text, Green, Black);
}

static bool flash_read_checked(uint32_t address, uint8_t *buffer, uint32_t length)
{
    w25q32_flash_read(buffer, address, length, 2);
    if (w25q32_last_operation_ok()) return true;
    usart1_printf("FLASH READ ERROR\r\n");
    return false;
}

/* 沿用原存储地址和 CRLF 记录格式；一次写整条记录，避免两次擦写。 */
static bool flash_write_checked(uint32_t address, const uint8_t *buffer, uint32_t length)
{
    w25q32_flash_autosector_write_with_erase((u8 *)buffer, address, length, 2);
    if (w25q32_last_operation_ok()) return true;
    usart1_printf("FLASH SAVE ERROR\r\n");
    return false;
}

static void reset_nrf_stream(void)
{
    uint8_t discard[128];
    unsigned int i;
    nrf_line_used = 0;
    nrf_discard_line = false;
    candidate_count = 0;
    candidate_conflict = false;
    for (i = 0; i < USART1_RING_BUFFER_LEN / sizeof(discard); ++i)
        if (read_ringbuffer_count_for_usart1(&usart2_receive_buffer, (char *)discard, sizeof(discard)) == 0U) break;
    nrf_last_dropped = get_ringbuffer_dropped_for_usart1(&usart2_receive_buffer);
}

/* 等待时仅采样/锁存按键，返回主循环后再执行，避免插入新的NRF命令。 */
static void delay_with_button_sampling(unsigned int milliseconds)
{
    TickType_t start = xTaskGetTickCount();
    TickType_t duration = pdMS_TO_TICKS(milliseconds);
    TickType_t step = pdMS_TO_TICKS(10);
    if (step == 0) step = 1;
    for (;;) {
        TickType_t elapsed, remaining;
        button_sample_events();
        elapsed = xTaskGetTickCount() - start;
        if (elapsed >= duration) break;
        remaining = duration - elapsed;
        vTaskDelay(remaining < step ? remaining : step);
    }
}

/* 保留原来的三个 150 ms 等待窗口。只接受本轮发送后收齐的应答。 */
static bool nrf_configure(const char *command, unsigned int expected)
{
    unsigned int attempt;
    reset_nrf_stream();
    nrf_ack_flags = 0;
    usart2_printf("%s", command);
    for (attempt = 0; attempt < 3U; ++attempt) {
        delay_with_button_sampling(150);
        process_nrf_input();
        if ((nrf_ack_flags & expected) != 0U) return true;
    }
    return false;
}

static bool set_rssi(int8_t value, bool persist)
{
    char command[32], text[8];
    if (value > 0 || value < -105) { usart1_printf("rssi out of range\r\n"); return false; }
    sprintf(command, "rssi_set=%d\r\n", (int)value);
    if (!nrf_configure(command, ACK_RSSI)) { usart1_printf("rssi set timeout\r\n"); return false; }
    sprintf(text, "%d", (int)value);
    show_setting(161, text);
    usart1_printf("\r\n%s\r\n", text);
    if (persist && !flash_write_checked(FLASH_RSSI_ADDRESS, (const uint8_t *)&value, 1)) return false;
    usart1_dma_tx_send((u8 *)rssi_ok, sizeof(rssi_ok));
    return true;
}

static bool set_feature(const char *feature, bool persist)
{
    uint8_t scratch[8];
    char command[48], record[19];
    size_t length = strlen(feature);
    if ((length != 12U && length != 14U && length != 16U) ||
        !app_hex_bytes(feature, scratch, length / 2U)) return false;
    sprintf(command, "feature_value_set=%s\r\n", feature);
    if (!nrf_configure(command, ACK_FEATURE)) { usart1_printf("feature set timeout\r\n"); return false; }
    show_setting(181, feature);
    usart1_printf("\r\n%s\r\n", feature);
    if (persist) {
        sprintf(record, "%s\r\n", feature);
        if (!flash_write_checked(FLASH_FEATURE_ADDRESS, (const uint8_t *)record, (uint32_t)length + 2U)) return false;
    }
    usart1_dma_tx_send((u8 *)feature_ok, sizeof(feature_ok));
    return true;
}

static bool id_settings_valid(void)
{
    return id_length >= 1U && id_length <= APP_ID_MAX_BYTES && (!cmd_select_flag || old_command >= 0x83U);
}

static void show_id_setting(uint8_t value, bool old)
{
    char text[5];
    unsigned int x = old ? 121U : 85U;
    sprintf(text, "0X%02x", value);
    lcd_fill_color(x, 261, x + 32U, 277, Black);
    display_ascii_string(x, 261, 8, (u8 *)text, Green, Black);
}

static void request_scan(void)
{
    if (!id_settings_valid()) { usart1_printf("ONLY_LEN SET ERROR\r\n"); return; }
    usart2_printf("start_scanning\r\n");
}

/* 确认键：RSSI -> 特征值 -> 长度/旧命令 -> 开始扫描，保持原顺序。
 * 各成功标志是本次调用局部变量，失败后再次按键不会继承旧成功状态。 */
static void configure_from_flash(void)
{
    uint8_t value, record[32] = {0};
    char feature[17];
    bool rssi_valid = false, feature_valid = false, length_valid = false, old_valid = !cmd_select_flag;
    cmd_set_flag_from_buttom = false;
    if (flash_read_checked(FLASH_RSSI_ADDRESS, &value, 1)) rssi_valid = set_rssi((int8_t)value, false);
    if (flash_read_checked(FLASH_FEATURE_ADDRESS, record, 31) &&
        app_field((const char *)record, "", "\r\n", feature, sizeof(feature))) feature_valid = set_feature(feature, false);
    if (flash_read_checked(FLASH_ID_LENGTH_ADDRESS, record, 3) && record[1] == '\r' && record[2] == '\n' &&
        record[0] >= 1U && record[0] <= APP_ID_MAX_BYTES) {
        id_length = record[0];
        length_valid = true;
        show_id_setting(id_length, false);
    }
    if (cmd_select_flag) {
        if (flash_read_checked(FLASH_OLD_COMMAND_ADDRESS, record, 3) && record[1] == '\r' &&
            record[2] == '\n' && record[0] >= 0x83U) {
            old_command = record[0];
            old_valid = true;
            show_id_setting(old_command, true);
            lcd_fill_color(5, 414, 317, 426, Black);
        } else {
            display_ascii_string(5, 414, 6, (u8 *)"OLD_ONLY_LEN SET ERROR", Red, Black);
        }
    } else lcd_fill_color(121, 261, 153, 277, Black);
    display_ascii_string(165, 261, 8, (u8 *)(decoding_setting_flag ? "PLAINTEXT" : "CIPHERTEXT"), Green, Black);
    if (rssi_valid && feature_valid && length_valid && old_valid) request_scan();
    else usart1_printf("FLASH CONFIG ERROR: check settings, then press confirm again\r\n");
}

static void set_id_from_text(const char *command)
{
    char text[3];
    uint8_t value, record[3];
    if (!app_field(command, "only_len_set=", "\r\n", text, sizeof(text)) ||
        !app_hex_bytes(text, &value, 1) || (value < 0x83U && (value == 0U || value > APP_ID_MAX_BYTES))) {
        usart1_printf("ONLY_LEN SET ERROR\r\n");
        return;
    }
    record[0] = value; record[1] = '\r'; record[2] = '\n';
    if (!flash_write_checked(value < 0x83U ? FLASH_ID_LENGTH_ADDRESS : FLASH_OLD_COMMAND_ADDRESS, record, 3)) return;
    if (value < 0x83U) id_length = value;
    else old_command = value;
    show_id_setting(value, value >= 0x83U);
    usart1_printf("%s\r\n", command);
}

static void set_project_from_text(const char *command)
{
    char project[27], record[30] = {0};
    size_t length;
    if (!app_field(command, "prj_set=", "\r\n", project, sizeof(project))) {
        usart1_printf("NAME TOO LONG OR EMPTY\r\n");
        return;
    }
    length = strlen(project);
    memcpy(record, project, length);
    memcpy(record + length, "\r\n", 2);
    if (!flash_write_checked(FLASH_PROJECT_ADDRESS, (const uint8_t *)record, sizeof(record))) return;
    lcd_fill_color(1, 1, 320, 52, Black);
    display_ascii_string((320U - length * 12U) / 2U, 14, 12, (u8 *)project, LimeGreen, Black);
    usart1_printf("PRO_NAME = %s\r\n", project);
}

static void process_config_request(const ConfigRequest *request)
{
    const uint8_t *frame = request->data;
    const char *text = (const char *)frame;
    size_t length = request->length;
    char feature[17];
    if (strncmp(text, "prj_set=", 8) == 0) { set_project_from_text(text); return; }
    if (!usart2_cmd_set_flag) { usart1_printf("configuration locked while scanning\r\n"); return; }
    if (frame[0] == 0x7f) {
        if (!app_binary_valid(frame, length) || frame[3] != 0 || frame[4] != 0) return;
        if (length == 7U && frame[2] == 0x81) set_rssi((int8_t)frame[5], true);
        else if (length >= 12U && length <= 14U && frame[2] == 0x82) {
            hex_arr_to_str((u8 *)(frame + 5), (u32)length - 6U, feature);
            set_feature(feature, true);
        }
    } else if (strncmp(text, "only_len_set=", 13) == 0) set_id_from_text(text);
    else if (strcmp(text, "mask_raw_data\r\n") == 0) usart2_printf("mask_raw_data\r\n");
    else if (strcmp(text, "use_default_settings\r\n") == 0) usart2_printf("use_default_settings\r\n");
    else if (strcmp(text, "start_scanning\r\n") == 0) request_scan();
}

/* 同一批中所有完整广播都必须属于同一产品；MAC 和 SN 成对校验。 */
static void handle_nrf_line(const char *line)
{
    AppAdvertisement ad;
    if (strstr(line, "rssi_set_ok") != NULL) nrf_ack_flags |= ACK_RSSI;
    if (strstr(line, "feature_value = 0x") != NULL) nrf_ack_flags |= ACK_FEATURE;
    if (strstr(line, "mask_raw_data_ok") != NULL) { show_setting(201, "MASK ON"); raw_mode = true; }
    if (strstr(line, "default_settings_ok") != NULL) {
        show_setting(221, "USE DEFAULT");
        if (raw_mode) {
            show_setting(241, "START SCANNING");
            lcd_fill_color(85, 357, 317, 373, Black);
        }
    }
    if (strstr(line, "plaintext_set_ok") != NULL && decoding_setting_flag) {
        lcd_fill_color(5, 81, 77, 97, Black);
        display_ascii_string(5, 81, 8, (u8 *)"CHIP ID :", Silver, Black);
        lcd_fill_color(165, 261, 317, 277, Black);
        display_ascii_string(165, 261, 8, (u8 *)"PLAINTEXT", Green, Black);
    }
    if (strstr(line, "cipnertext_set_ok") != NULL && !decoding_setting_flag) {
        lcd_fill_color(5, 81, 77, 97, Black);
        display_ascii_string(5, 81, 8, (u8 *)"MAC     :", Silver, Black);
        lcd_fill_color(165, 261, 317, 277, Black);
        display_ascii_string(165, 261, 8, (u8 *)"CIPHERTEXT", Green, Black);
    }
    if (strstr(line, "start_scanning_ok") != NULL) {
        show_setting(241, "START SCANNING");
        lcd_fill_color(85, 357, 317, 373, Black);
        usart2_cmd_set_flag = false;
        raw_mode = false;
    }
    if (raw_mode) { usart1_printf("%s", line); return; }
    if (usart2_cmd_set_flag || pc_waiting || strstr(line, "[DECRYPT_") == NULL) return;
    if (!app_parse_advertisement(line, &ad)) { candidate_conflict = true; return; }
    if (candidate_count == 0U) { candidate = ad; candidate_count = 1; }
    else if (!app_same_product(&candidate, &ad)) candidate_conflict = true;
    else if (candidate_count < 2U) ++candidate_count;
}

static void process_nrf_input(void)
{
    char bytes[128];
    unsigned int count, i, batch, dropped;
    USART2_REVEIVE_OK = false;
    dropped = get_ringbuffer_dropped_for_usart1(&usart2_receive_buffer);
    if (dropped != nrf_last_dropped) { reset_nrf_stream(); return; }
    for (batch = 0; batch < USART1_RING_BUFFER_LEN / sizeof(bytes); ++batch) {
        count = read_ringbuffer_count_for_usart1(&usart2_receive_buffer, bytes, sizeof(bytes));
        if (count == 0U) break;
        if (get_ringbuffer_dropped_for_usart1(&usart2_receive_buffer) != nrf_last_dropped) {
            reset_nrf_stream(); return;
        }
        for (i = 0; i < count; ++i) {
            if (nrf_discard_line) {
                if (bytes[i] == '\n') nrf_discard_line = false;
                continue;
            }
            if (nrf_line_used >= sizeof(nrf_line) - 1U || bytes[i] == 0) {
                nrf_line_used = 0;
                nrf_discard_line = bytes[i] != '\n';
                candidate_conflict = true;
                continue;
            }
            nrf_line[nrf_line_used++] = bytes[i];
            nrf_line[nrf_line_used] = 0;
            if (bytes[i] == '\n') {
                if (nrf_line_used >= 2U && nrf_line[nrf_line_used - 2U] == '\r') handle_nrf_line(nrf_line);
                else candidate_conflict = true;
                nrf_line_used = 0;
            }
        }
    }
    if (candidate_conflict) {
        candidate_count = 0;
        /* 未完成的下一行继续保留，完整候选全部作废。 */
        candidate_conflict = false;
        display_ascii_string(8, 389, 6, (u8 *)"Invalid or duplicated MAC/SN", Red, Black);
    }
}

static void show_product(const AppAdvertisement *ad, uint16_t color)
{
    if (decoding_setting_flag) display_ascii_string(85, 83, 6, (u8 *)ad->id, color, Black);
    else display_ascii_string(85, 81, 8, (u8 *)ad->id, color, Black);
    display_ascii_string(85, 101, 8, (u8 *)ad->sn, color, Black);
    display_ascii_string(85, 121, 8, (u8 *)ad->channel, color, Black);
    display_ascii_string(85, 141, 8, (u8 *)ad->rssi, color, Black);
    lcd_fill_color(2, 283, 318, 349, color);
    display_ascii_string(46, 300, 16, (u8 *)ad->sn, Black, color);
}

static void start_report(void)
{
    uint8_t command = cmd_select_flag ? old_command : id_length;
    if (candidate_count < 2U || pc_waiting) return;
    candidate_count = 0;
    report_length = app_build_packet(&candidate, id_length, command, report_packet, sizeof(report_packet));
    if (!id_settings_valid() || report_length == 0U) { usart1_printf("MAC/SN LENGTH OR VALUE ERROR\r\n"); return; }
    reporting = candidate;
    usart2_disable();
    reset_nrf_stream();
    taskENTER_CRITICAL();
    pc_expected_command = command;
    pc_ack_received = false;
    pc_waiting = true;
    time3_count1 = 0;
    pc_reback_msg_to_usart1_timeout = false;
    taskEXIT_CRITICAL();
    last_retry_slot = 0;
    time3_run(true);
    lcd_fill_color(85, 81, 318, 158, Black);
    lcd_fill_color(8, 389, 317, 401, Black);
    show_product(&reporting, Yellow);
    usart1_dma_tx_send(report_packet, (u16)report_length);
    lcd_fill_color(210, 440, 318, 477, Black);
    /* 黄色保留 500 ms；让出 CPU，使 PC 回复和定时器可以正常运行。 */
    delay_with_button_sampling(500);
}

static void process_report_result(void)
{
    bool acknowledged, timeout, waiting;
    uint32_t ticks;
    taskENTER_CRITICAL();
    waiting = pc_waiting;
    acknowledged = pc_ack_received;
    timeout = pc_reback_msg_to_usart1_timeout;
    ticks = time3_count1;
    taskEXIT_CRITICAL();
    if (!waiting) return;
    if (acknowledged || timeout) {
        show_product(&reporting, timeout ? Red : Green);
        time3_run(false);
        taskENTER_CRITICAL();
        pc_waiting = false;
        pc_ack_received = false;
        time3_count1 = 0;
        pc_reback_msg_to_usart1_timeout = false;
        taskEXIT_CRITICAL();
        reset_nrf_stream();
        usart2_enable();
        return;
    }
    /* 每 4 个 500 ms 计数重发一次；同一个时间片只发一次，不在 0 时重发。 */
    if (app_retry_due(ticks, &last_retry_slot)) {
        usart1_dma_tx_send(report_packet, (u16)report_length);
    }
}

static void update_progress(void)
{
    if (progress_x + 2U <= 318U) {
        lcd_fill_color(progress_x, 430, progress_x + 2U, 436, Yellow);
        progress_x += 2U;
    } else { progress_x = 3; lcd_fill_color(3, 430, 318, 436, Black); }
}

void usart2_receive_print_task(void *pvParameters)
{
    ConfigRequest request;
    TickType_t last_cycle = xTaskGetTickCount() - pdMS_TO_TICKS(100);
    (void)pvParameters;
    for (;;) {
        TickType_t now;
        /* 按键每10ms采样；广播批处理保持原100ms观察窗口。 */
        button_process_events();
        now = xTaskGetTickCount();
        if ((TickType_t)(now - last_cycle) >= pdMS_TO_TICKS(100)) {
            last_cycle = now;
            process_nrf_input();
            start_report();
            process_report_result();
            time3_update_display();
            if (xQueueReceive(config_queue, &request, 0) == pdPASS) process_config_request(&request);
            else if (usart2_cmd_set_flag && cmd_set_flag_from_buttom) configure_from_flash();
            update_progress();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *task_name)
{
    (void)task;
    board_fatal_error(task_name);
}
