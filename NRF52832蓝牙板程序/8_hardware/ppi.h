#ifndef __PPI_H__
#define __PPI_H__

#include <stdint.h>

	
//*****************************************************************************************************************
void ppi_channel_setup(uint8_t ch, uint32_t eep_addr, uint32_t tep_addr);
void ppi_channel_enable(uint8_t ch);
void ppi_channel_disable(uint8_t ch);	
void ppi_channel_fork_setup(uint8_t ch, uint32_t fork_tep_addr);

#endif


