#ifndef MYPORT_H
#define MYPORT_H

#define dispatch isr_pendsv
#define xPortSysTickHandler isr_systick

#define portNVIC_INT_CTRL_REG      (*((volatile uint32_t *)0xe000ed04))
#define portNVIC_PENDSVSET_BIT     (1UL << 28UL)
#define portYIELD()                (portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT)

#include <stdint.h>

typedef uint32_t StackType_t;
typedef void (*TaskFunction_t)(void *);

typedef struct tskTaskControlBlock
{
    StackType_t *pxTopOfStack;
} TCB_t;

typedef TCB_t * TaskHandle_t;

extern TaskHandle_t pxCurrentTCB;

StackType_t * pxPortInitialiseStack( StackType_t * pxTopOfStack,
                                     TaskFunction_t pxCode,
                                     void * pvParameters );

void StartFirstTask( void );
void dispatch( void );

#endif // MYPORT_H