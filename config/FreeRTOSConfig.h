#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H


#include<stdint.h>
#include<stdbool.h>

bool cycle_counter_init(void);
uint32_t cycle_counter_get(void);

#define configPRIO_BITS                                         4U

#define configUSE_PREEMPTION                                    1
#define configUSE_TIME_SLICING                                  1
#define configCPU_CLOCK_HZ                                      16000000UL
#define configTICK_RATE_HZ                                      1000
#define configMAX_PRIORITIES                                    5
#define configMINIMAL_STACK_SIZE                                128
#define configTOTAL_HEAP_SIZE                                   (16*1024)
#define configMAX_TASK_NAME_LEN                                 20
#define configTICK_TYPE_WIDTH_IN_BITS                           TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD                                 1
#define configUSE_TASK_NOTIFICATIONS                            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES                   1
#define configUSE_MUTEXES                                       1
#define configUSE_RECURSIVE_MUTEXES                             0
#define configUSE_COUNTING_SEMAPHORES                           1
#define configUSE_ALTERNATIVE_API                               0
#define configUSE_QUEUE_SETS                                    0
#define configQUEUE_REGISTRY_SIZE                               8
#define configUSE_NEWLIB_REENTRANT                              0
#define configENABLE_BACKWARD_COMPATIBILITY                     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS                 1
#define configSTACK_DEPTH_TYPE                                  __UINT16_TYPE__
#define configMESSAGE_BUFFER_LENGTH_TYPE                        __UINT16_TYPE__
#define configHEAP_CLEAR_MEMORY_ON_FREE                         0
#define configUSE_MINI_LIST_ITEM                                1
#define configUSE_POSIX_ERRNO                                   0
#define configUSE_SB_COMPLETED_CALLBACK                         0
#define configSUPPORT_STATIC_ALLOCATION                         0
#define configSUPPORT_DYNAMIC_ALLOCATION                        1
#define configKERNEL_PROVIDED_STATIC_MEMORY                     0
#define configAPPLICATION_ALLOCATED_HEAP                        1
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP               0
#define configENABLE_HEAP_PROTECTOR                             1

#define configKERNEL_INTERRUPT_PRIORITY                        ( 15U << ( 8U - configPRIO_BITS ) )

#define configMAX_SYSCALL_INTERRUPT_PRIORITY                   ( 5U  << ( 8U - configPRIO_BITS ) )

#define configMAX_API_CALL_INTERRUPT_PRIORITY                  configMAX_SYSCALL_INTERRUPT_PRIORITY 
#define configUSE_DAEMON_TASK_STARTUP_HOOK                      0
#define configUSE_MALLOC_FAILED_HOOK                            1
#define configUSE_IDLE_HOOK                                     0
#define configUSE_TICK_HOOK                                     0
#define configCHECK_FOR_STACK_OVERFLOW                          2
#define configUSE_TRACE_FACILITY                                1
#define configGENERATE_RUN_TIME_STATS                           1
#define configUSE_STATS_FORMATTING_FUNCTIONS                    0
#define configUSE_APPLICATION_TASK_TAG                          0

#define INCLUDE_vTaskPrioritySet                                1
#define INCLUDE_vTaskDelete                                     1
#define INCLUDE_vTaskSuspend                                    1
#define INCLUDE_xTaskResumeFromISR                              1
#define INCLUDE_xTaskDelayUntil                                 1
#define INCLUDE_uxTaskPriorityGet                               1
#define INCLUDE_vTaskDelay                                      1
#define INCLUDE_xTaskGetSchedulerState                          1
#define INCLUDE_xTaskGetCurrentTaskHandle                       1
#define INCLUDE_xTaskGetIdleTaskHandle                          1
#define INCLUDE_xTaskGetHandle                                  0
#define INCLUDE_eTaskGetState                                   1
#define INCLUDE_xTimerPendFunctionCall                          1
#define INCLUDE_xTaskAbortDelay                                 0
#define INCLUDE_uxTaskGetStackHighWaterMark                     1

#define configASSERT(x)                      \
    do                                       \
    {                                        \
        if( ( x ) == 0 )                     \
        {                                    \
            taskDISABLE_INTERRUPTS();        \
            __asm volatile( "bkpt #0" );     \
            for( ;; );                       \
        }                                    \
    } while( 0 )                             

#define configUSE_TIMERS                                        1
#define configTIMER_TASK_PRIORITY                               ( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH                                10
#define configTIMER_TASK_STACK_DEPTH                            256

#define configUSE_CO_ROUTINES                                   0
#define configUSE_EVENT_GROUPS                                  1
#define configUSE_STREAM_BUFFERS                                0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION                 1
#define configUSE_TICKLESS_IDLE                                 0
#define configSTATS_BUFFER_MAX_LENGTH                           0xFFFF

#define configCHECK_HANDLER_INSTALLATION                        1

#define vPortSVCHandler                                         SVC_Handler
#define xPortPendSVHandler                                      PendSV_Handler
#define xPortSysTickHandler                                     SysTick_Handler

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()    cycle_counter_init()

#define portGET_RUN_TIME_COUNTER_VALUE()            cycle_counter_get()

#endif