/** PPI 硬件事件到任务的连接；当前应用独占通道 0、1、2。 */
#include "ppi.h"
#include "nrf_ppi.h"

void ppi_channel_setup(uint8_t ch, uint32_t event_address, uint32_t task_address)
{
    nrf_ppi_channel_endpoint_setup((nrf_ppi_channel_t)ch, event_address, task_address);
}

void ppi_channel_enable(uint8_t ch)
{
    nrf_ppi_channel_enable((nrf_ppi_channel_t)ch);
}

void ppi_channel_disable(uint8_t ch)
{
    nrf_ppi_channel_disable((nrf_ppi_channel_t)ch);
}

void ppi_channel_fork_setup(uint8_t ch, uint32_t task_address)
{
    nrf_ppi_fork_endpoint_setup((nrf_ppi_channel_t)ch, task_address);
}
