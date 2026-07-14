#ifndef SYSTEM_PANIC_H
#define SYSTEM_PANIC_H


typedef enum
{
    PANIC_ASSERT = 0,
    PANIC_MALLOC_FAILED,
    PANIC_STACK_OVERFLOW,
    PANIC_CYCLE_COUNTER_UNAVAILABLE

} panic_reason_t;

void system_panic(panic_reason_t reason);

#endif