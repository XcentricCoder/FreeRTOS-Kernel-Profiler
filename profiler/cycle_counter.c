#include <stdint.h>
#include "cycle_counter.h"

#define COREDEBUG_DEMCR                     (*(volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL                            (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT                          (*(volatile uint32_t *)0xE0001004)
#define COREDEBUG_DEMCR_TRCENA              (1UL<<24)
#define DWT_CTRL_CYCCNTENA                  (1UL<<0)
#define DWT_CTRL_NOCYCCNT                   (1UL<<25)

void cycle_counter_init(void)
{
    COREDEBUG_DEMCR |= COREDEBUG_DEMCR_TRCENA;  // Trace enable for DWT access

    if(DWT_CTRL & DWT_CTRL_NOCYCCNT)
    {
        return;                                 // Verify whether Cycle Counter is supported or not
    }
    
    DWT_CYCCNT = 0;                             // Initialize the Cycle Counter Value to 0

    DWT_CTRL |= DWT_CTRL_CYCCNTENA;             // Enable the count on Cycle Counter

}

uint32_t cycle_counter_get(void)
{
    return DWT_CYCCNT;
}