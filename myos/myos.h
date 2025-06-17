#ifndef MYOS_H
#define MYOS_H

#include <stdint.h>

typedef uint32_t StackType_t;
typedef void (*TaskFunction_t)(void *);

extern volatile StackType_t *pxCurrentTCB;
extern volatile StackType_t *pxNextTCB;

StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters);
void StartFirstTask(void);
void dispatch(void);

#endif
