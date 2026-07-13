#include <stdint.h>
#include "cycle_counter.h"
#include "profiler.h"

uint32_t first_elapsed;
uint32_t second_elapsed;
uint32_t third_elapsed;

profiler_record_t test_record;

int main(void)
{
    cycle_counter_init();

    profiler_init(&test_record);

    //Run 100 iterations

    profiler_start(&test_record);

    

    for(volatile uint32_t i =0;i<100;i++)
    {

    }

    profiler_stop(&test_record);

    first_elapsed = test_record.last_cycles;

    //Run 200 iterations

    profiler_start(&test_record);

    for(volatile uint32_t i =0;i<200;i++)
    {

    }

    profiler_stop(&test_record);

    second_elapsed = test_record.last_cycles;

    //Run 300 iterations

    profiler_start(&test_record);

    for(volatile uint32_t i =0;i<300;i++)
    {

    }

    profiler_stop(&test_record);

    third_elapsed = test_record.last_cycles;


    while(1)
    {

    }

}