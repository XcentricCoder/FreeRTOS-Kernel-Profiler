#include <stdint.h>
#include "cycle_counter.h"

volatile uint32_t start;
volatile uint32_t stop;
volatile uint32_t elapsed;

int main(void)
{
    cycle_counter_init();

    start = cycle_counter_get();

    for(volatile uint32_t i =0; i<1000; i++){

    }

    stop = cycle_counter_get();

    elapsed = stop - start;

    while(1)
    {

    }

}