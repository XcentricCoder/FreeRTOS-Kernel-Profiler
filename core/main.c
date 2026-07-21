#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "cycle_counter.h"
#include "system_panic.h"
#include "task_profiler.h"



volatile uint32_t task_a_counter = 0U;
volatile uint32_t task_b_counter = 0U;


static void task_a( void * pvParameters )
{
    ( void ) pvParameters;

    for( ;; )
    {
        task_a_counter++;

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