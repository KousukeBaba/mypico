#include "myport.h"
#include <stdio.h>

#define portINITIAL_XPSR  ( 0x01000000 )

void task_exit_handler(void)
{
	printf("ERROR: Task returned unexpectedly! System halted.\n");
	while (1);
}

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
					 TaskFunction_t pxCode,
					 void * pvParameters )
{
	pxTopOfStack--;
	*pxTopOfStack = portINITIAL_XPSR;
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) pxCode;
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) task_exit_handler;
	pxTopOfStack -= 5;
	*pxTopOfStack = ( StackType_t ) pvParameters;
	pxTopOfStack -= 8;

	return pxTopOfStack;
}

void StartFirstTask( void )
{
	__asm volatile (

		"	.syntax unified\n"
		"	ldr  r2, pxCurrentTCBConst2\n"
		"	ldr  r3, [r2]\n"
		"	ldr  r0, [r3]\n"
		"	adds r0, #32\n"

		"	msr  psp, r0\n"

		"	movs r0, #2\n"
		"	msr  CONTROL, r0\n"
		"	isb\n"

		"	pop  {r0-r5}\n"
		"	mov  lr, r5\n"
		"	pop  {r3}\n"
		"	pop  {r2}\n"
		"	cpsie i\n"
		"	bx   r3\n"
		"	.align 4\n"
		"pxCurrentTCBConst2: .word pxCurrentTCB\n"
	);
}

void dispatch( void )
{
	__asm volatile (

		"	.syntax unified\n"

		"	mrs r0, psp\n"
		"	ldr r3, pxCurrentTCBConst\n"
		"	ldr r2, [r3]\n"
		"	subs r0, r0, #32\n"
		"	str r0, [r2]\n"
		"	stmia r0!, {r4-r7}\n"
		"	mov r4, r8\n"
		"	mov r5, r9\n"
		"	mov r6, r10\n"
		"	mov r7, r11\n"
		"	stmia r0!, {r4-r7}\n"

		"	push {r3, r14}\n"
		"	cpsid i\n"
		"	bl vTaskSwitchContext\n"
		"	cpsie i\n"

		"	pop {r2, r3}\n"
		"	ldr r1, [r2]\n"
		"	ldr r0, [r1]\n"
		"	adds r0, r0, #16\n"
		"	ldmia r0!, {r4-r7}\n"
		"	mov r8, r4\n"
		"	mov r9, r5\n"
		"	mov r10, r6\n"
		"	mov r11, r7\n"
		
		"	msr psp, r0\n"
		"	subs r0, r0, #32\n"
		"	ldmia r0!, {r4-r7}\n"
		"	bx r3\n"
		"	.align 4\n"
		"pxCurrentTCBConst: .word pxCurrentTCB\n"
	);
}
