#ifndef TASK_PROFILER_H
#define TASK_PROFILER_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"


typedef struct{
    TaskHandle_t task;
    const char *name;
    uint64_t runtime_cycles;
    uint32_t switch_count;
    uint16_t cpu_usage;

}task_profiler_snapshot_t;

void task_profiler_init(void);
uint32_t task_profiler_get_task_count(void);
const char *task_profiler_get_task_name(uint32_t index);
uint64_t task_profiler_get_runtime_cycles(uint32_t index);
uint64_t task_profiler_get_total_runtime_cycles(void);
uint16_t task_profiler_cpu_usage(uint32_t index);


BaseType_t task_profiler_get_snapshot(
    uint32_t index,
    task_profiler_snapshot_t *snapshot
);

#endif