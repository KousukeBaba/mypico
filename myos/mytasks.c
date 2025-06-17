#include "mytasks.h"
#define MAX_TASKS 2

static TCB_t tcb_array[MAX_TASKS];
static StackType_t *stack_ptrs[MAX_TASKS];
static uint8_t task_index = 0;

TaskHandle_t pxCurrentTCB = NULL;

void vTaskSwitchContext(void)
{
	task_index = (task_index + 1) % MAX_TASKS;
	pxCurrentTCB = &tcb_array[task_index];
}

TaskHandle_t create_task(StackType_t *stack_mem,
                         uint32_t stack_size,
                         TaskFunction_t entry,
                         void *arg)
{
	if (task_index >= MAX_TASKS) return NULL;

	StackType_t *top = stack_mem + stack_size;
	top = pxPortInitialiseStack(top, entry, arg);

tcb_array[task_index].pxTopOfStack = top;
stack_ptrs[task_index] = stack_mem;
return &tcb_array[task_index++];
}

void start_scheduler(void)
{
	pxCurrentTCB = &tcb_array[0];
	StartFirstTask();
}