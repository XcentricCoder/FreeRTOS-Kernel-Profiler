#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

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
#define configUSE_APPLICATION_TASK_TAG                          1
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

#define configKERNEL_INTERRUPT_PRIORITY                        ( 15U << ( 8U - __NVIC_PRIO_BITS ) )

#define configMAX_SYSCALL_INTERRUPT_PRIORITY                   ( 5U  << ( 8U - __NVIC_PRIO_BITS ) )

#define configMAX_API_CALL_INTERRUPT_PRIORITY                  configMAX_SYSCALL_INTERRUPT_PRIORITY 
#define configUSE_DAEMON_TASK_STARTUP_HOOK                      0
#define configUSE_MALLOC_FAILED_HOOK                            1
#define configUSE_IDLE_HOOK                                     0
#define configUSE_TICK_HOOK                                     0
#define configCHECK_FOR_STACK_OVERFLOW                          2
#define configUSE_TRACE_FACILITY                                1
#define configGENERATE_RUN_TIME_STATS

#endif