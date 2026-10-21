#ifndef TEST_STM32F4XX_H
#define TEST_STM32F4XX_H
#include <stdint.h>
extern uint32_t test_primask;
extern void test_irq_restore(uint32_t value);
static inline uint32_t __get_PRIMASK(void) { return test_primask; }
static inline void __disable_irq(void) { test_primask = 1U; }
static inline void __set_PRIMASK(uint32_t value) { test_irq_restore(value); }
#endif
