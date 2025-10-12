#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

#define SYST_CSR   (*(volatile uint32_t *)0xE000E010) // SysTick Control and Status
#define SYST_RVR   (*(volatile uint32_t *)0xE000E014) // Reload Value
#define SYST_CVR   (*(volatile uint32_t *)0xE000E018) // Current Value
#define SYST_CALIB (*(volatile uint32_t *)0xE000E01C) // Calibration Value

#define SYSTICK_ENABLE     (1UL << 0UL)
#define SYSTICK_TICKINT    (1UL << 1UL) //systick有効化
#define SYSTICK_CLKSOURCE  (1UL << 2UL)

#define CPU_CLOCK_HZ       125000000UL
#define TICK_RATE_HZ       1000UL

void systick_init(void);
void isr_systick(void);
void systick_delay_ms(uint32_t ms);

#endif // SYSTICK_H
