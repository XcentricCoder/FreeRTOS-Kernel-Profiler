#include "system_panic.h"


volatile panic_reason_t g_panic_reason;

void system_panic(panic_reason_t reason)
{
    g_panic_reason = reason;

    __asm volatile ("cpsid i");
    __asm volatile ("bkpt #0");

    for(;;)
    {

    }
}