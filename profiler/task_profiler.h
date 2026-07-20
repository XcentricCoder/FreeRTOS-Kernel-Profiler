#ifndef TASK_PROFILER_H
#define TASK_PROFILER_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

typedef struct{
    TaskHandle_t task;
    uint64_t total_cycles;
    uint32_t last_start_cycles;
    uint32_t switch_count;

}task_profiler_record_t;


void task_profiler_init(void);
BaseType_t task_profiler_register_task(TaskHandle_t task);
void task_profiler_switch_in(TaskHandle_t task);
void task_profiler_switch_out(TaskHandle_t task);

uint32_t task_profiler_get_task_count(void);
const task_profiler_record_t *task_profiler_get_record(uint32_t index);
const char *task_profiler_get_task_name(uint32_t index);
uint64_t task_profiler_get_runtime_cycles(uint32_t index);
uint64_t task_profiler_get_total_runtime_cycles(void);
uint16_t task_profiler_cpu_usage(uint32_t index);

typedef struct{
    TaskHandle_t task;
    const char *name;
    uint64_t runtime_cycles;
    uint32_t switch_count;
    uint16_t cpu_usage;

}task_profiler_snapshot_t;

BaseType_t task_profiler_get_snapshot(
    uint32_t index,
    task_profiler_snapshot_t *snapshot
);

#endif