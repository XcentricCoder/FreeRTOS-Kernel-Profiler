#include "profiler.h"
#include "cycle_counter.h"

void profiler_init(profiler_record_t *record)
{
    record->start_cycle = 0;
    record->last_cycles = 0;
    record->run_count = 0;
    record->max_cycles = 0;
    record->total_cycles = 0;

}
void profiler_start(profiler_record_t *record)
{
    record->start_cycle = cycle_counter_get();

}
void profiler_stop(profiler_record_t *record)
{
    uint32_t stopped = cycle_counter_get();
    uint32_t elapsed = stopped - record->start_cycle;

    record->last_cycles = elapsed;
    record->total_cycles += elapsed;

    record->run_count++;

    if(elapsed > record->max_cycles)
    {
        record->max_cycles = elapsed;
    }


}
