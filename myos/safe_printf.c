#include <stdio.h>
#include <stdarg.h>

void safe_printf(const char *fmt, ...) {
    __asm volatile("cpsid i");  // 割り込み禁止

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);

    __asm volatile("cpsie i");  // 割り込み許可
}
