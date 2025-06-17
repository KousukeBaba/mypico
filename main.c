#include "mytasks.h"
#include "myport.h"
#include "systick.h"
#include "pico/stdlib.h"
#include <stdio.h>

#define STACK_SIZE 128

StackType_t stack1[STACK_SIZE];
StackType_t stack2[STACK_SIZE];

void task1(void *arg)
{
	printf("Task 1 start\n");
	while (1) { printf("1\n"); }
}

void task2(void *arg)
{
	printf("Task 2 start\n");
	while (1) { printf ("2\n"); }
}

int main()
{
	stdio_init_all();
	systick_init();

	create_task(stack1, STACK_SIZE, task1, NULL);
	create_task(stack2, STACK_SIZE, task2, NULL);
	start_scheduler();

	while (1);
}
