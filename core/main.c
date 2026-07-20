#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "cycle_counter.h"
#include "system_panic.h"
#include "task_profiler.h"
//#include "timer.h"


volatile uint32_t task_a_counter = 0U;
volatile uint32_t task_b_counter = 0U;

volatile uint32_t profiler_task_count = 0;

volatile uint64_t profiler_task0_cycles = 0;
volatile uint64_t profiler_task1_cycles = 0;
volatile uint64_t profiler_task2_cycles = 0;

volatile uint16_t profiler_task0_cpu = 0;
volatile uint16_t profiler_task1_cpu = 0;
volatile uint16_t profiler_task2_cpu = 0;

volatile const char *profiler_task0_name = NULL;
volatile const char *profiler_task1_name = NULL;
volatile const char *profiler_task2_name = NULL;


static void task_a( void * pvParameters )
{
    ( void ) pvParameters;

    for( ;; )
    {
        task_a_counter++;
        
         if((task_a_counter % 100000U) == 0U){
         profiler_task_count = task_profiler_get_task_count();

        if(profiler_task_count > 0U)
        {
            profiler_task0_name = task_profiler_get_task_name(0);
            profiler_task0_cycles = task_profiler_get_runtime_cycles(0);
            profiler_task0_cpu = task_profiler_cpu_usage(0);
        }

        if(profiler_task_count > 1U)
        {
            profiler_task1_name = task_profiler_get_task_name(1);
            profiler_task1_cycles = task_profiler_get_runtime_cycles(1);
            profiler_task1_cpu = task_profiler_cpu_usage(1);
        }

        if(profiler_task_count > 2U)
        {
            profiler_task2_name = task_profiler_get_task_name(2);
            profiler_task2_cycles = task_profiler_get_runtime_cycles(2);
            profiler_task2_cpu = task_profiler_cpu_usage(2);
        }
    }

    }
}

static void task_b(void *pvParameters)
{
    (void)pvParameters;

    for(;;)
    {
        task_b_counter++;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}


int main( void )
{
    BaseType_t task_a_status;
    BaseType_t task_b_status;

    if( !cycle_counter_init() )
    {
        system_panic( PANIC_CYCLE_COUNTER_UNAVAILABLE );
    }

    task_profiler_init();

    task_a_status = xTaskCreate(
        task_a,
        "TaskA",
        128,
        NULL,
        1,
        NULL
    );

    if( task_a_status != pdPASS )
    {
        system_panic( PANIC_MALLOC_FAILED );
    }

    task_b_status = xTaskCreate(
        task_b,
        "TaskB",
        128,
        NULL,
        1,
        NULL
    );

    if( task_b_status != pdPASS )
    {
        system_panic( PANIC_MALLOC_FAILED );
    }

    vTaskStartScheduler();

    for( ;; )
    {
    }
}