#include "FreeRTOS.h"
#include "task.h"

#include "system_panic.h"
#include "../profiler/cycle_counter.h"


void vApplicationMallocFailedHook( void )
{
    system_panic(PANIC_MALLOC_FAILED);
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName )
{
    ( void ) xTask;
    ( void ) pcTaskName;

    system_panic(PANIC_STACK_OVERFLOW);
}

void vApplicationGetRandomHeapCanary(portPOINTER_SIZE_TYPE * pxHeapCanary )
{
    uintptr_t stack_address;
    uint32_t cycles;

    stack_address = (uintptr_t) &stack_address;
    cycles = cycle_counter_get();

    *pxHeapCanary = (portPOINTER_SIZE_TYPE)(stack_address ^ cycles ^ 0xA5C39E71UL);

    if(*pxHeapCanary == 0u)
    {
        *pxHeapCanary = (portPOINTER_SIZE_TYPE)(0xA5C39E71UL);
    }
}