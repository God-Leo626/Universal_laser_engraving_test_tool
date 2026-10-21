/* Event-order model of SDK 15.2 nrfx_uarte.c. This does not emulate radio or silicon timing. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "uarte_52832_dma.h"
#include "timer.h"
#include "ppi.h"
#include "my_printf.h"

static unsigned checks;
#define CHECK(expr) do { ++checks; if (!(expr)) { fprintf(stderr, "UART FAIL line %d: %s\n", __LINE__, #expr); exit(1); } } while (0)
uint32_t mock_ipsr;
mock_timer_t mock_timer;
static nrfx_uarte_event_handler_t uart_handler;
static nrfx_timer_event_handler_t timer_handler;
static uint8_t *rx_buffer;
static size_t rx_length, rx_amount;
static bool receiver_running, stop_pending, tx_busy, tx_immediate, delay_completes_tx;
static unsigned rx_starts, stops, init_calls, delay_calls;
static const uint8_t *tx_buffer;
static size_t tx_length;
static bool ppi_enabled[3];

nrfx_err_t nrfx_uarte_init(const nrfx_uarte_t *uart, const nrfx_uarte_config_t *config, nrfx_uarte_event_handler_t handler)
{
    (void)uart;
    CHECK(config->interrupt_priority == 6 && config->baudrate == 115200);
    CHECK(config->pseltxd == 6 && config->pselrxd == 8 && config->hwfc == 0);
    uart_handler = handler; ++init_calls; return NRFX_SUCCESS;
}
nrfx_err_t nrfx_uarte_rx(const nrfx_uarte_t *uart, uint8_t *data, size_t length)
{
    (void)uart;
    CHECK(!receiver_running && !stop_pending && rx_length == 0);
    CHECK(length == 255);
    rx_buffer = data; rx_length = length; rx_amount = 0; receiver_running = true; ++rx_starts;
    return NRFX_SUCCESS;
}
void nrfx_uarte_rx_abort(const nrfx_uarte_t *uart)
{
    (void)uart; CHECK(receiver_running && !stop_pending); stop_pending = true; ++stops;
}
bool nrfx_uarte_tx_in_progress(const nrfx_uarte_t *uart) { (void)uart; return tx_busy; }
static void tx_done(void)
{
    nrfx_uarte_event_t event = {0}; event.type = NRFX_UARTE_EVT_TX_DONE;
    tx_busy = false; uart_handler(&event, NULL);
}
nrfx_err_t nrfx_uarte_tx(const nrfx_uarte_t *uart, const uint8_t *data, size_t length)
{
    (void)uart; CHECK(!tx_busy && length > 0 && length <= 255);
    tx_busy = true; tx_buffer = data; tx_length = length;
    if (tx_immediate) tx_done();
    return NRFX_SUCCESS;
}
uint32_t nrfx_uarte_event_address_get(const nrfx_uarte_t *uart, int evt) { (void)uart; return (uint32_t)evt; }
void timer1_init(uint32_t timeout, uint8_t priority, nrfx_timer_event_handler_t handler)
{
    CHECK(timeout == 2000 && priority == 6); timer_handler = handler;
}
void timer1_stop(void) { }
void timer1_clear(void) { }
void timer1_stop_and_clear(void) { }
uint32_t timer1_task_addr_get(nrf_timer_task_t task) { return (uint32_t)task; }
void ppi_channel_setup(uint8_t ch, uint32_t event, uint32_t task)
{
    CHECK(ch < 3);
    if (ch == 2) CHECK(event == NRF_UARTE_EVENT_RXTO && task == NRF_TIMER_TASK_CAPTURE1);
}
void ppi_channel_enable(uint8_t ch) { CHECK(ch < 3); ppi_enabled[ch] = true; }
void ppi_channel_disable(uint8_t ch) { CHECK(ch < 3); ppi_enabled[ch] = false; }
void ppi_channel_fork_setup(uint8_t ch, uint32_t task) { (void)ch; (void)task; }
void nrf_delay_us(uint32_t us)
{
    CHECK(us == 10); ++delay_calls;
    if (delay_completes_tx) tx_done();
}
static void receive(size_t amount, uint8_t value)
{
    CHECK(receiver_running && amount <= 255);
    rx_amount = amount; memset(rx_buffer, value, amount);
}
static void full_callback(void)
{
    nrfx_uarte_event_t event = {0};
    CHECK(rx_amount == rx_length);
    rx_length = 0; /* SDK clears length before full RX callback. */
    event.type = NRFX_UARTE_EVT_RX_DONE; event.data.rxtx.bytes = rx_amount;
    uart_handler(&event, NULL);
}
static void stop_event(void)
{
    CHECK(stop_pending && ppi_enabled[2]);
    receiver_running = false; stop_pending = false;
    mock_timer.cc[1] = 0; /* PPI captures RXTO even when SDK sends no callback. */
    if (rx_length != 0) {
        nrfx_uarte_event_t event = {0};
        rx_length = 0;
        event.type = NRFX_UARTE_EVT_RX_DONE; event.data.rxtx.bytes = rx_amount;
        uart_handler(&event, NULL);
    }
}
int main(void)
{
    uint8_t data[256]; size_t length = 999; bool damaged = true;
    CHECK(!uart_dma_read(data, sizeof(data), &length, &damaged));
    CHECK(uart_dma_send(data, 1) == UART_DMA_ERR_INVALID_STATE);
    uarte_dma_init(); uarte_dma_init(); CHECK(init_calls == 1 && rx_starts == 1);
    receive(17, 0x42);
    timer_handler(NRF_TIMER_EVENT_COMPARE0, NULL);
    CHECK(stops == 1 && !ppi_enabled[0] && !ppi_enabled[1]);
    CHECK(!uart_dma_read(data, sizeof(data), &length, &damaged));
    stop_event();
    CHECK(!uart_dma_read(data, 254, &length, &damaged));
    CHECK(uart_dma_read(data, sizeof(data), &length, &damaged));
    CHECK(length == 17 && !damaged && data[16] == 0x42 && rx_starts == 2);
    receive(255, 0xA5); full_callback();
    timer_handler(NRF_TIMER_EVENT_COMPARE0, NULL); /* stale timeout must not STOP twice */
    CHECK(stops == 2 && !uart_dma_read(data, sizeof(data), &length, &damaged));
    stop_event();
    CHECK(uart_dma_read(data, sizeof(data), &length, &damaged) && length == 255 && !damaged);
    CHECK(data[254] == 0xA5 && rx_starts == 3);
    receive(2, 1);
    nrfx_uarte_event_t error = {0}; error.type = NRFX_UARTE_EVT_ERROR;
    rx_length = 0; uart_handler(&error, NULL); /* SDK error branch clears length */
    CHECK(!uart_dma_read(data, sizeof(data), &length, &damaged));
    stop_event();
    CHECK(uart_dma_read(data, sizeof(data), &length, &damaged) && damaged && length == 0);
    receive(0, 0); timer_handler(NRF_TIMER_EVENT_COMPARE0, NULL); stop_event();
    CHECK(uart_dma_read(data, sizeof(data), &length, &damaged) && !damaged && length == 0);
    receive(1, 0xCC); timer_handler(NRF_TIMER_EVENT_COMPARE0, NULL); stop_event();
    CHECK(uart_dma_read(data, sizeof(data), &length, &damaged) && data[0] == 0xCC);
    memset(data, 0x55, sizeof(data));
    CHECK(uart_dma_send(data, 256) == UART_DMA_ERR_INVALID_PARAM);
    CHECK(uart_dma_send(data, 0) == UART_DMA_ERR_INVALID_PARAM);
    CHECK(uart_dma_send(NULL, 1) == UART_DMA_ERR_INVALID_PARAM);
    CHECK(uart_dma_send(data, 255) == UART_DMA_OK && uart_dma_is_tx_busy());
    memset(data, 0x66, sizeof(data)); CHECK(tx_buffer[0] == 0x55 && tx_buffer[254] == 0x55);
    CHECK(uart_dma_send(data, 1) == UART_DMA_ERR_BUSY && tx_buffer[0] == 0x55);
    tx_done(); tx_immediate = true;
    CHECK(uart_dma_send(data, 1) == UART_DMA_OK && !uart_dma_is_tx_busy());
    mock_ipsr = 1; CHECK(uart_dma_send(data, 1) == UART_DMA_ERR_INVALID_STATE); mock_ipsr = 0;
    tx_immediate = false; delay_completes_tx = true;
    print("%s", "hello"); CHECK(tx_length == 5 && memcmp(tx_buffer, "hello", 5) == 0);
    delay_completes_tx = false; delay_calls = 0;
    char huge[400]; memset(huge, 'z', sizeof(huge) - 1); huge[399] = 0;
    print("%s", huge);
    CHECK(tx_length == 255);
    CHECK(delay_calls == 5000);
    CHECK(tx_busy);
    print("changed"); CHECK(tx_buffer[0] == 'z' && tx_buffer[254] == 'z'); tx_done();
    printf("UART event regression PASS: %u checks\n", checks);
    return 0;
}
