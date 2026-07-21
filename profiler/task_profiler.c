#include "task_profiler.h"
#include "profiler_hooks.h"
#include <stdint.h>
#include "cycle_counter.h"

#define TASK_PROFILER_MAX_TASKS 8U

typedef struct{
    TaskHandle_t task;
    uint64_t total_cycles;
    uint32_t last_start_cycles;
    uint32_t switch_count;

}task_profiler_record_t;

static task_profiler_record_t records[TASK_PROFILER_MAX_TASKS];
static uint32_t registered_task_count = 0U;

static BaseType_t task_profiler_register_task(TaskHandle_t task);
static void task_profiler_switch_in(TaskHandle_t task);
static void task_profiler_switch_out(TaskHandle_t task);
static const task_profiler_record_t *task_profiler_get_record(uint32_t index);

void task_profiler_init(void){

    for(uint32_t i =0; i<TASK_PROFILER_MAX_TASKS;i++){
        records[i].last_start_cycles=0;
        records[i].switch_count=0;
        records[i].task=NULL;
        records[i].total_cycles=0;
    }

    registered_task_count=0U;
}

static BaseType_t task_profiler_register_task(TaskHandle_t task){

    if(task == NULL){
        return pdFAIL;
    }

    uint32_t i=0;
    while(i< registered_task_count){
        if(records[i].task == task){
            break;
        }
        i++;
    }

    if (i < registered_task_count){
    return pdPASS;
    }

    if (registered_task_count >= TASK_PROFILER_MAX_TASKS){
        return pdFAIL;
    }

        records[registered_task_count].last_start_cycles=0;
        records[registered_task_count].switch_count=0;
        records[registered_task_count].task=task;
        records[registered_task_count].total_cycles=0;

        registered_task_count++;

    return pdPASS;


}

static task_profiler_record_t *task_profiler_find_record(TaskHandle_t task){
    uint32_t i =0;
    while(i< registered_task_count){
        if (records[i].task==task){
            return &records[i];
        }
        i++;
    }
    return NULL;
}


static void task_profiler_switch_in(TaskHandle_t task){
    task_profiler_record_t *record = task_profiler_find_record(task);
    if (record == NULL){
        return;
    }

    uint32_t now= cycle_counter_get();
    record->last_start_cycles =now;
    record->switch_count++;

}

static void task_profiler_switch_out(TaskHandle_t task){
    task_profiler_record_t *record = task_profiler_find_record(task);

    if(record == NULL){
        return ;
    }

    uint32_t end = cycle_counter_get();
    record->total_cycles += (end - record->last_start_cycles);
}

void profiler_trace_task_switched_in(void){
    TaskHandle_t task =xTaskGetCurrentTaskHandle();

    if(task == NULL){
        return;
    }

    if (task_profiler_register_task(task) != pdPASS)
    {
       return;
    }

    task_profiler_switch_in(task);


}

void profiler_trace_task_switched_out(void){
    TaskHandle_t task =xTaskGetCurrentTaskHandle();

    task_profiler_switch_out(task);


}

uint32_t task_profiler_get_task_count(void){
    return registered_task_count;
}

static const task_profiler_record_t *task_profiler_get_record(uint32_t index){
    if(index >= registered_task_count){
        return NULL;
    }
    return &records[index];
}

const char *task_profiler_get_task_name(uint32_t index){
    if(index >= registered_task_count){
        return NULL;
    }
    const task_profiler_record_t *record = task_profiler_get_record(index);
    const char *name = pcTaskGetName(record->task);

    return name;
}

uint64_t task_profiler_get_runtime_cycles(uint32_t index){
    if(index >= registered_task_count){
        return 0;
    }
    const task_profiler_record_t *record = task_profiler_get_record(index);

    uint64_t runtime_cycles = record->total_cycles;

    TaskHandle_t current_task = xTaskGetCurrentTaskHandle();

    if (record->task == current_task){
        runtime_cycles+= (cycle_counter_get() - record->last_start_cycles);
    }

    return runtime_cycles;

}


uint64_t task_profiler_get_total_runtime_cycles(void){

    uint64_t total_runtime_cycles =0;

    for(uint32_t i =0; i<registered_task_count; i++){
        total_runtime_cycles += task_profiler_get_runtime_cycles(i);
    }

    return total_runtime_cycles;

}

uint16_t task_profiler_cpu_usage(uint32_t index){
    uint64_t task_index = task_profiler_get_runtime_cycles(index);
    uint64_t task_total = task_profiler_get_total_runtime_cycles();

    if(task_total == 0){
        return 0;
    }

    uint16_t cpu_usage = (task_index * 10000ULL)/task_total;

    return cpu_usage;
}

BaseType_t task_profiler_get_snapshot(uint32_t index, task_profiler_snapshot_t *snapshot){
    if(snapshot == NULL){
        return pdFAIL;
    }

    uint32_t now;
    uint64_t task_runtime = 0U;
    uint64_t total_runtime = 0U;
    uint32_t switch_count;
    uint32_t task_count;
    TaskHandle_t current_task;
    TaskHandle_t requested_task;

    taskENTER_CRITICAL();

    task_count = registered_task_count;

    if(index >= task_count){
        taskEXIT_CRITICAL();
        return pdFAIL;
    }

    now = cycle_counter_get();
    current_task = xTaskGetCurrentTaskHandle();
    requested_task = records[index].task;

    switch_count = records[index].switch_count;

    for (uint32_t i =0; i< task_count; i++){

        uint64_t runtime = records[i].total_cycles;

        if (records[i].task == current_task){

            runtime += (uint32_t)(now - records[i].last_start_cycles);
        }

        total_runtime += runtime;

        if(i==index){
            task_runtime = runtime;
        }

    }



    taskEXIT_CRITICAL();



    snapshot->task = requested_task;
    snapshot->switch_count = switch_count;
    snapshot->name = pcTaskGetName(snapshot->task);
    snapshot->runtime_cycles = task_runtime;

    if (total_runtime == 0U){
        snapshot->cpu_usage = 0U;
    }
    else
    {
        snapshot->cpu_usage = (uint16_t)((task_runtime * 10000ULL)/ total_runtime);
    }

    return pdPASS;
};