#ifndef TEST_NRF_GPIO_H
#define TEST_NRF_GPIO_H
#include "nrfx_uarte.h"
#define NRF_GPIO_PIN_PULLUP 1
static inline void nrf_gpio_cfg_input(uint32_t pin, int pull) { (void)pin; (void)pull; }
#endif
