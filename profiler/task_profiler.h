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

#endif