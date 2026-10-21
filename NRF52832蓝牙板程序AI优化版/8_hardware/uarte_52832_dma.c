/**
 * UARTE0 EasyDMA 收发。
 * RXDRDY 经 PPI 清零并启动 TIMER1，2 ms 空闲后只请求异步 STOPRX。
 * 所有接收完成都由 nrfx 回调发布，主循环取走数据后才重启接收。
 * 中断内没有打印、忙等或对 nrfx 内部状态的手工修改。
 */
#include "uarte_52832_dma.h"
#include "timer.h"
#include "ppi.h"
#include "nrfx_uarte.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include <string.h>

#define UART_IRQ_PRIORITY 6u
#define PPI_RX_CLEAR 0u
#define PPI_RX_START 1u
#define PPI_RX_STOPPED 2u
#define RX_STOP_NOT_OBSERVED UINT32_MAX

static const nrfx_uarte_t m_uarte = NRFX_UARTE_INSTANCE(0);
__ALIGN(4) static uint8_t m_rx_buf[UART_RX_BUF_SIZE];
__ALIGN(4) static uint8_t m_tx_buf[UART_TX_BUF_SIZE];
static bool m_initialized;
static volatile bool m_rx_stopping;
static volatile bool m_rx_done;
static volatile bool m_rx_damaged;
static volatile size_t m_rx_length;

static void rx_start(void)
{
    m_rx_stopping = false;
    m_rx_done = false;
    m_rx_damaged = false;
    m_rx_length = 0;
    timer1_stop_and_clear();
    nrf_timer_event_clear(NRF_TIMER1, NRF_TIMER_EVENT_COMPARE0);
    ppi_channel_enable(PPI_RX_CLEAR);
    ppi_channel_enable(PPI_RX_START);
    APP_ERROR_CHECK(nrfx_uarte_rx(&m_uarte, m_rx_buf, sizeof(m_rx_buf)));
}

static void rx_request_stop(void)
{
    if (m_rx_stopping) return;
    ppi_channel_disable(PPI_RX_CLEAR);
    ppi_channel_disable(PPI_RX_START);
    timer1_stop_and_clear();
    /* RX 满/ERROR 时 SDK 已清软件长度，后续 RXTO 不再产生 RX_DONE。
     * 因此借 PPI CH2 将 RXTO 捕获到空闲 CC1：硬件确实停止后才能重启。
     * 此时 TIMER1 已停在 0，CAPTURE1 不可能仍为 UINT32_MAX。
     */
    nrf_timer_cc_write(NRF_TIMER1, NRF_TIMER_CC_CHANNEL1, RX_STOP_NOT_OBSERVED);
    m_rx_stopping = true;
    __DMB();
    nrfx_uarte_rx_abort(&m_uarte);
}

static void uarte_evt_handler(const nrfx_uarte_event_t *event, void *context)
{
    (void)context;
    switch (event->type) {
    case NRFX_UARTE_EVT_RX_DONE:
        m_rx_length = event->data.rxtx.bytes;
        if (m_rx_length > sizeof(m_rx_buf)) {
            m_rx_length = 0;
            m_rx_damaged = true;
        }
        /* 满缓冲回调先于 RXTO；空闲中止则由 RXTO 产生本回调。 */
        rx_request_stop();
        __DMB();
        m_rx_done = true;
        break;
    case NRFX_UARTE_EVT_ERROR:
        m_rx_damaged = true;
        m_rx_length = 0;
        rx_request_stop();
        __DMB();
        m_rx_done = true;
        break;
    case NRFX_UARTE_EVT_TX_DONE:
        /* nrfx 在回调之前清除 TX 长度，直接查询驱动可避免 busy 竞态。 */
        break;
    default:
        break;
    }
}

static void timer_evt_handler(nrf_timer_event_t event, void *context)
{
    (void)context;
    if (event == NRF_TIMER_EVENT_COMPARE0) rx_request_stop();
}

void uarte_dma_init(void)
{
    nrfx_uarte_config_t config = NRFX_UARTE_DEFAULT_CONFIG;
    if (m_initialized) return;
    config.pseltxd = TX_PIN_NUMBER;
    config.pselrxd = RX_PIN_NUMBER;
    config.pselcts = NRF_UARTE_PSEL_DISCONNECTED;
    config.pselrts = NRF_UARTE_PSEL_DISCONNECTED;
    config.hwfc = NRF_UARTE_HWFC_DISABLED;
    config.parity = NRF_UARTE_PARITY_EXCLUDED;
    config.baudrate = NRF_UARTE_BAUDRATE_115200;
    config.interrupt_priority = UART_IRQ_PRIORITY;
    config.p_context = NULL;
    APP_ERROR_CHECK(nrfx_uarte_init(&m_uarte, &config, uarte_evt_handler));
    nrf_gpio_cfg_input(RX_PIN_NUMBER, NRF_GPIO_PIN_PULLUP);
    timer1_init(UART_RX_TIMEOUT_US, UART_IRQ_PRIORITY, timer_evt_handler);

    /* 应用独占 PPI 0/1/2 和 TIMER1 CC0/CC1；S132 保留的 PPI 从 17 开始。 */
    ppi_channel_disable(PPI_RX_CLEAR);
    ppi_channel_disable(PPI_RX_START);
    ppi_channel_disable(PPI_RX_STOPPED);
    ppi_channel_setup(PPI_RX_CLEAR,
        nrfx_uarte_event_address_get(&m_uarte, NRF_UARTE_EVENT_RXDRDY),
        timer1_task_addr_get(NRF_TIMER_TASK_CLEAR));
    ppi_channel_setup(PPI_RX_START,
        nrfx_uarte_event_address_get(&m_uarte, NRF_UARTE_EVENT_RXDRDY),
        timer1_task_addr_get(NRF_TIMER_TASK_START));
    ppi_channel_setup(PPI_RX_STOPPED,
        nrfx_uarte_event_address_get(&m_uarte, NRF_UARTE_EVENT_RXTO),
        timer1_task_addr_get(NRF_TIMER_TASK_CAPTURE1));
    ppi_channel_enable(PPI_RX_STOPPED);
    m_initialized = true;
    rx_start();
}

bool uart_dma_read(uint8_t *data, size_t capacity, size_t *length, bool *damaged)
{
    if (!m_initialized || data == NULL || capacity < sizeof(m_rx_buf) ||
        length == NULL || damaged == NULL || !m_rx_done || !m_rx_stopping) return false;
    if (nrf_timer_cc_read(NRF_TIMER1, NRF_TIMER_CC_CHANNEL1) == RX_STOP_NOT_OBSERVED) return false;
    __DMB();
    *length = m_rx_length;
    *damaged = m_rx_damaged;
    memcpy(data, m_rx_buf, *length);
    /* 先复制，再重启，主循环解析期间 DMA 不会改写调用方手中的数据。 */
    rx_start();
    return true;
}

uart_dma_err_t uart_dma_send(const uint8_t *data, uint16_t length)
{
    nrfx_err_t result;
    if (!m_initialized || __get_IPSR() != 0) return UART_DMA_ERR_INVALID_STATE;
    if (data == NULL || length == 0 || length > sizeof(m_tx_buf)) return UART_DMA_ERR_INVALID_PARAM;
    if (nrfx_uarte_tx_in_progress(&m_uarte)) return UART_DMA_ERR_BUSY;
    memcpy(m_tx_buf, data, length);
    result = nrfx_uarte_tx(&m_uarte, m_tx_buf, length);
    if (result == NRFX_SUCCESS) return UART_DMA_OK;
    return result == NRFX_ERROR_BUSY ? UART_DMA_ERR_BUSY : UART_DMA_ERR_INVALID_STATE;
}

bool uart_dma_is_tx_busy(void)
{
    return m_initialized && nrfx_uarte_tx_in_progress(&m_uarte);
}
