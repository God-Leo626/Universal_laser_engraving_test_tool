#ifndef TEST_NRFX_UARTE_H
#define TEST_NRFX_UARTE_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define __ALIGN(n) __attribute__((aligned(n)))
#define __DMB() __asm__ volatile ("" ::: "memory")
extern uint32_t mock_ipsr;
#define __get_IPSR() mock_ipsr
typedef int nrfx_err_t;
#define NRFX_SUCCESS 0
#define NRFX_ERROR_BUSY 1
#define NRF_UARTE_PSEL_DISCONNECTED UINT32_MAX
#define NRF_UARTE_HWFC_DISABLED 0
#define NRF_UARTE_PARITY_EXCLUDED 0
#define NRF_UARTE_BAUDRATE_115200 115200
#define NRFX_UARTE_DEFAULT_CONFIG {0}
typedef struct { unsigned id; } nrfx_uarte_t;
#define NRFX_UARTE_INSTANCE(n) {n}
typedef struct {
    uint32_t pseltxd, pselrxd, pselcts, pselrts;
    int hwfc, parity, baudrate;
    uint8_t interrupt_priority;
    void *p_context;
} nrfx_uarte_config_t;
typedef enum { NRFX_UARTE_EVT_RX_DONE, NRFX_UARTE_EVT_ERROR, NRFX_UARTE_EVT_TX_DONE } nrfx_uarte_evt_type_t;
typedef struct { size_t bytes; uint8_t *p_data; } mock_transfer_t;
typedef struct {
    nrfx_uarte_evt_type_t type;
    union { mock_transfer_t rxtx; struct { uint32_t error_mask; mock_transfer_t rxtx; } error; } data;
} nrfx_uarte_event_t;
typedef void (*nrfx_uarte_event_handler_t)(const nrfx_uarte_event_t *, void *);
enum { NRF_UARTE_EVENT_RXDRDY = 10, NRF_UARTE_EVENT_RXTO = 11 };
nrfx_err_t nrfx_uarte_init(const nrfx_uarte_t *, const nrfx_uarte_config_t *, nrfx_uarte_event_handler_t);
nrfx_err_t nrfx_uarte_rx(const nrfx_uarte_t *, uint8_t *, size_t);
void nrfx_uarte_rx_abort(const nrfx_uarte_t *);
nrfx_err_t nrfx_uarte_tx(const nrfx_uarte_t *, const uint8_t *, size_t);
bool nrfx_uarte_tx_in_progress(const nrfx_uarte_t *);
uint32_t nrfx_uarte_event_address_get(const nrfx_uarte_t *, int);
#endif
