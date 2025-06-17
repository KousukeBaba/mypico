#ifndef MYTASK_H
#define MYTASK_H

#include <stdint.h>
#include "myport.h"
#include "pico/stdlib.h"
#include <stdio.h>

TaskHandle_t create_task(StackType_t *stack_mem,
                         uint32_t stack_size,
                         TaskFunction_t entry,
                         void *arg);

void start_scheduler(void);
void vTaskSwitchContext(void);

#endif // MYTASK_H