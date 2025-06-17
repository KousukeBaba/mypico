#include "systick.h"
#include "myport.h"


void isr_systick(void) {
    // ここで簡易スケジューラを実装するかも
    portYIELD();
}

void systick_init(void) {
    __asm volatile("cpsid i");  // IRQを無効化したまま最初のタスクまで保持
    SYST_CSR = 0UL;
    SYST_CVR = 0UL;

    SYST_RVR = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
    SYST_CSR = SYSTICK_CLKSOURCE | SYSTICK_TICKINT | SYSTICK_ENABLE;
}
