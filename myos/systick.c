#include "systick.h"
#include "myport.h"
#include "pico/stdlib.h"
#include "pico/time.h"

/*TICKINTだけ無効化： SYST_CSR &= ~SYSTICK_TICKINT; スレッド切り替え禁止(恐らく)
         　　有効化： SYST_CSR |= SYSTICK_TICKINT;
*/


void isr_systick(void) { //Context Switch
    portYIELD();
}

void systick_init(void) {
    __asm volatile("cpsid i");  // IRQを無効化したまま最初のタスクまで保持
    SYST_CSR = 0UL;
    SYST_CVR = 0UL;

    SYST_RVR = (CPU_CLOCK_HZ / TICK_RATE_HZ) - 1UL;
    SYST_CSR = SYSTICK_CLKSOURCE | SYSTICK_TICKINT | SYSTICK_ENABLE;
}

void systick_delay_ms(uint32_t ms) { //待機処理 sleep_msをスレッド内で呼ぶとhardfaultになる
    uint64_t start = time_us_64();
    while ((time_us_64() - start) < (uint64_t)ms * 1000) {
        tight_loop_contents();  // CPU待機
    }
}
