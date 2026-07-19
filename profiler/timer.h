#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>


typedef struct
{
    uint32_t start_cycle;
    uint32_t last_cycles;
    uint32_t total_cycles;
    uint32_t max_cycles;
    uint32_t run_count;

}   profiler_record_t;

void profiler_init(profiler_record_t *record);
void profiler_start(profiler_record_t *record);
void profiler_stop(profiler_record_t *record);


#endif