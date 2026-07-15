# Day 03 --- FreeRTOS Kernel Bring-Up and Scheduler Validation

**Project:** FreeRTOS Kernel Profiler\
**Target:** STM32F411RE / Cortex-M4F\
**Environment:** Bare metal, `arm-none-eabi-gcc`, custom linker script
and startup code\
**End-of-day status:** FreeRTOS scheduler running on physical hardware;
preemptive equal-priority time slicing verified.

------------------------------------------------------------------------

## 1. Objective

Day 03 moved the project from a bare-metal profiling foundation to a
working FreeRTOS execution platform.

Objectives completed:

-   Finalize `FreeRTOSConfig.h`.
-   Integrate the Cortex-M4F FreeRTOS port.
-   Integrate `heap_4`.
-   Provide an application-owned FreeRTOS heap.
-   Explicitly reserve MSP stack space in the linker script.
-   Add FreeRTOS hooks and a system panic path.
-   Provide minimum freestanding C memory functions.
-   Route SVC, PendSV and SysTick to FreeRTOS.
-   Build and link the complete kernel.
-   Start the scheduler on the STM32F411.
-   Validate MSP/PSP separation and time slicing using GDB.

------------------------------------------------------------------------

## 2. FreeRTOS Configuration

Important scheduler configuration:

``` c
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configCPU_CLOCK_HZ                      16000000UL
#define configTICK_RATE_HZ                      1000
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                128
```

Resulting scheduler configuration:

``` text
Preemptive scheduling       : enabled
Equal-priority time slicing : enabled
CPU clock                   : 16 MHz
Kernel tick                 : 1 kHz
Tick period                 : 1 ms
Maximum task priorities     : 5
```

At 16 MHz and 1 kHz:

``` text
16,000,000 / 1,000 = 16,000 CPU clocks per tick
```

### Interrupt priorities

For the STM32F411:

``` c
#define __NVIC_PRIO_BITS 4
```

FreeRTOS priority configuration:

``` c
#define configKERNEL_INTERRUPT_PRIORITY     ( 15U << ( 8U - __NVIC_PRIO_BITS ) )

#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( 5U << ( 8U - __NVIC_PRIO_BITS ) )
```

Therefore:

``` text
configKERNEL_INTERRUPT_PRIORITY      = 0xF0
configMAX_SYSCALL_INTERRUPT_PRIORITY = 0x50
```

### Profiler-related facilities

``` c
#define configUSE_TRACE_FACILITY                    1
#define configGENERATE_RUN_TIME_STATS               1
#define INCLUDE_uxTaskGetStackHighWaterMark         1
```

The DWT cycle counter is connected to the FreeRTOS run-time statistics
interface:

``` c
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()     cycle_counter_init()

#define portGET_RUN_TIME_COUNTER_VALUE()     cycle_counter_get()
```

------------------------------------------------------------------------

## 3. FreeRTOS Exception Routing

The configuration aliases the ARM port handlers to the names used by the
custom vector table:

``` c
#define vPortSVCHandler       SVC_Handler
#define xPortPendSVHandler    PendSV_Handler
#define xPortSysTickHandler   SysTick_Handler
```

Architecture:

``` text
Custom vector table
        |
        +--> SVC_Handler     --> FreeRTOS ARM_CM4F port
        +--> PendSV_Handler  --> FreeRTOS ARM_CM4F port
        +--> SysTick_Handler --> FreeRTOS ARM_CM4F port
```

ELF validation:

``` text
0800006e t Default_Handler
08002594 T SVC_Handler
0800287c T PendSV_Handler
080028e0 T SysTick_Handler
```

The scheduler exceptions are therefore routed to real FreeRTOS port
handlers rather than `Default_Handler`.

------------------------------------------------------------------------

## 4. Linker Script and MSP Reservation

The linker script now explicitly reserves a Main Stack Pointer region:

``` ld
_Min_Stack_Size = 0x800;
_estack = ORIGIN(RAM) + LENGTH(RAM);
_sstack = _estack - _Min_Stack_Size;
```

For the 128 KB SRAM:

``` text
RAM origin = 0x20000000
RAM end    = 0x20020000
_estack    = 0x20020000
```

The intended memory architecture is:

``` text
0x20020000  <- _estack / initial MSP
     |
     | MSP stack, grows downward
     |
_sstack
     |
     | FreeRTOS heap / globals / runtime data
     |
0x20000000
```

Before scheduler startup:

``` text
Reset_Handler --> MSP
main()         --> MSP
```

After scheduler startup:

``` text
Tasks          --> PSP
SVC            --> MSP
SysTick        --> MSP
PendSV         --> MSP
```

The linker protects the MSP reservation with:

``` ld
ASSERT(_eheap <= _sstack,
       "RAM overflow: heap overlaps MSP stack")
```

------------------------------------------------------------------------

## 5. Application-Owned FreeRTOS Heap

The configuration uses:

``` c
#define configAPPLICATION_ALLOCATED_HEAP 1
```

The application therefore provides `ucHeap` in:

``` text
core/freertos_heap.c
```

Definition:

``` c
uint8_t ucHeap[configTOTAL_HEAP_SIZE]
    __attribute__((section(".heap"), aligned(8)));
```

Configured heap size:

``` c
#define configTOTAL_HEAP_SIZE (16 * 1024)
```

The final symbol table showed:

``` text
200001c8 B ucHeap
```

`heap_4.c` manages this application-owned SRAM arena and provides:

``` text
pvPortMalloc()
vPortFree()
```

FreeRTOS task TCBs and task stacks are dynamically allocated from this
heap.

------------------------------------------------------------------------

## 6. Hooks and System Panic Path

Safety hooks enabled:

``` c
#define configUSE_MALLOC_FAILED_HOOK   1
#define configCHECK_FOR_STACK_OVERFLOW 2
```

Application hook implementations live in:

``` text
core/hooks.c
```

The failure policy is implemented in:

``` text
core/system_panic.c
```

Flow:

``` text
FreeRTOS detects failure
        |
        +--> vApplicationMallocFailedHook()
        |
        +--> vApplicationStackOverflowHook()
                     |
                     v
                system_panic(...)
                     |
                     v
             terminal panic state
```

The kernel detects the error. The application owns the target-specific
response.

------------------------------------------------------------------------

## 7. Minimal Freestanding C Runtime

The build uses:

``` text
-nostdlib
-ffreestanding
```

After linking the kernel, unresolved references appeared for:

``` text
memset
memcpy
```

Kernel examples:

``` text
tasks.c --> memset
queue.c --> memcpy
```

The project added:

``` text
core/memory.c
```

with minimal `memset` and `memcpy` implementations.

This satisfies the kernel's memory-operation requirements without
linking a complete hosted C runtime.

------------------------------------------------------------------------

## 8. Kernel Files Integrated

FreeRTOS files now explicitly compiled:

``` text
freertos/FreeRTOS-Kernel/tasks.c
freertos/FreeRTOS-Kernel/queue.c
freertos/FreeRTOS-Kernel/list.c
freertos/FreeRTOS-Kernel/timers.c
freertos/FreeRTOS-Kernel/event_groups.c
freertos/FreeRTOS-Kernel/portable/MemMang/heap_4.c
freertos/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c
```

Project platform files:

``` text
core/main.c
core/freertos_heap.c
core/system_panic.c
core/hooks.c
core/memory.c
profiler/cycle_counter.c
profiler/profiler.c
startup/startup_stm32f411.s
```

### Architectural files to track

``` text
startup/startup_stm32f411.s
```

Reset, C runtime initialization and vector table.

``` text
core/main.c
```

Application task creation and scheduler startup.

``` text
freertos/FreeRTOS-Kernel/tasks.c
```

Task state, ready lists, scheduler policy and `pxCurrentTCB`.

``` text
freertos/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c
```

SVC, SysTick, PendSV, PSP and physical Cortex-M context switching.

``` text
freertos/FreeRTOS-Kernel/portable/MemMang/heap_4.c
```

Dynamic kernel memory allocation.

``` text
linker/stm32f411.ld
```

Physical FLASH and SRAM layout.

Mental ownership model:

``` text
startup.s  --> gets the processor into the C environment
main.c     --> creates OS objects and starts the scheduler
tasks.c    --> decides WHO runs
port.c     --> makes the CPU actually switch
heap_4.c   --> provides runtime kernel memory
```

------------------------------------------------------------------------

## 9. Successful Full Kernel Build

Final memory usage:

``` text
Memory region         Used Size  Region Size  %age Used
FLASH:                 10788 B       512 KB      2.06%
RAM:                   16840 B       128 KB     12.85%
```

ELF section summary:

``` text
text    = 10780 bytes
data    = 8 bytes
bss     = 16828 bytes
total   = 27616 bytes
```

The BSS increase is expected because the 16 KB `ucHeap` is part of the
SRAM layout.

Important final symbols:

``` text
08000296 T xTaskCreate
0800060c T vTaskStartScheduler
08001f1c T pvPortMalloc
200001c8 B ucHeap
```

This also confirmed that required kernel code survived `--gc-sections`.

------------------------------------------------------------------------

## 10. Scheduler Validation Test

Two equal-priority tasks were created:

``` c
static void task_a(void *pvParameters)
{
    for (;;)
    {
        task_a_counter++;
    }
}

static void task_b(void *pvParameters)
{
    for (;;)
    {
        task_b_counter++;
    }
}
```

Both tasks:

-   use priority 1,
-   remain permanently READY,
-   never call `vTaskDelay`,
-   never block,
-   never voluntarily yield.

Therefore both tasks executing proves tick-driven preemptive time
slicing.

The tested path is:

``` text
SysTick
   |
   v
kernel tick
   |
   v
time-slice decision
   |
   v
PendSV
   |
   v
context switch
```

------------------------------------------------------------------------

## 11. Exact Current Execution Path

``` text
RESET
  |
  v
Cortex-M reads vector[0]
  |
  +--> MSP = _estack = 0x20020000
  |
  v
Cortex-M reads vector[1]
  |
  +--> PC = Reset_Handler
  |
  v
Reset_Handler
  |
  +--> copy .data FLASH -> SRAM
  +--> zero .bss
  |
  v
main()
  |
  +--> cycle_counter_init()
  |       |
  |       +--> enable DWT access
  |       +--> clear DWT_CYCCNT
  |       +--> enable CYCCNT
  |
  +--> xTaskCreate(Task A)
  |       |
  |       +--> prvCreateTask()
  |       +--> pvPortMalloc()
  |       +--> heap_4 / ucHeap
  |       +--> allocate TCB
  |       +--> allocate stack
  |       +--> fill stack with 0xA5
  |       +--> pxPortInitialiseStack()
  |       +--> create synthetic initial context
  |       +--> add Task A to ready list
  |
  +--> xTaskCreate(Task B)
  |       |
  |       +--> same creation path
  |
  v
vTaskStartScheduler()
  |
  +--> create Idle task
  +--> create Timer service task
  +--> choose first ready task
  |
  v
xPortStartScheduler()
  |
  +--> configure SysTick
  +--> configure PendSV/SysTick priorities
  |
  v
SVC
  |
  v
SVC_Handler
  |
  +--> read pxCurrentTCB
  +--> load first task's saved stack pointer
  +--> restore initial task context
  +--> establish PSP task execution
  |
  v
Task A / Thread mode / PSP
  |
  | approximately 1 ms
  v
SysTick exception
  |
  +--> hardware saves R0-R3, R12, LR, PC, xPSR
  |    on the interrupted task PSP
  |
  v
SysTick_Handler / Handler mode / MSP
  |
  v
xTaskIncrementTick()
  |
  +--> increment kernel tick
  +--> process delayed tasks/timeouts
  +--> detect equal-priority ready task
  +--> request PendSV
  |
  v
PendSV_Handler / Handler mode / MSP
  |
  +--> read PSP
  +--> save R4-R11 to outgoing task stack
  +--> save PSP in outgoing TCB
  |
  v
vTaskSwitchContext()
  |
  +--> select next ready task
  +--> change pxCurrentTCB
  |
  v
PendSV_Handler
  |
  +--> load incoming task saved PSP
  +--> restore R4-R11
  +--> exception return
  |
  v
Hardware restores R0-R3, R12, LR, PC, xPSR
  |
  v
Task B / Thread mode / PSP
  |
  | approximately 1 ms
  v
SysTick -> PendSV -> Task A -> ...
```

Core context-switch model:

``` text
save outgoing task
        |
        v
change pxCurrentTCB
        |
        v
restore incoming task
```

------------------------------------------------------------------------

## 12. Synthetic Initial Task Context

A newly created task has never run.

`pxPortInitialiseStack()` constructs a synthetic saved context so the
normal context-restore path can start it.

Conceptually:

``` text
Task stack
+-----------------------------+
| xPSR = Thumb state          |
| PC   = task entry function  |
| LR   = task return handler  |
| R12                         |
| R3                          |
| R2                          |
| R1                          |
| R0   = pvParameters         |
| R11                         |
| R10                         |
| R9                          |
| R8                          |
| R7                          |
| R6                          |
| R5                          |
| R4                          |
+-----------------------------+
             ^
             |
        pxTopOfStack
```

The task stack is prepared to look as if the task had already been
interrupted.

For the first switch-in, this synthetic context is restored.

After the task has run and been switched out, its stack contains a real
saved execution context.

------------------------------------------------------------------------

## 13. GDB Scheduler Validation

Physical-board execution produced:

``` text
task_a_counter = 1529854
task_b_counter = 1529907
```

The difference was only 53 increments.

Both tasks are permanently ready and have the same priority. Neither
yields or blocks.

This verifies equal-priority scheduler time slicing.

GDB stopped at:

``` text
0x080000ae in task_b
core/main.c:31
task_b_counter++;
```

Task B was physically executing.

------------------------------------------------------------------------

## 14. MSP and PSP Validation

Live processor state:

``` text
msp = 0x2001ffe0
psp = 0x20000630
sp  = 0x20000630
```

The linker defines:

``` text
_estack = 0x20020000
```

Current MSP position:

``` text
0x20020000 - 0x2001FFE0 = 0x20 bytes
```

The MSP remains near the top of SRAM in the reserved exception-stack
region.

GDB decoded the PSP as:

``` text
0x20000630 <ucHeap+1128>
```

Since:

``` text
ucHeap = 0x200001C8
```

then:

``` text
0x20000630 - 0x200001C8 = 0x468 = 1128
```

The live task stack pointer is physically inside the application-owned
FreeRTOS heap.

Proven path:

``` text
heap_4
  |
  v
ucHeap
  |
  v
task stack allocation
  |
  v
PSP actively points into task stack
```

------------------------------------------------------------------------

## 15. CONTROL Register Validation

Measured state:

``` text
CONTROL = 0x2
```

Cortex-M CONTROL bits:

``` text
CONTROL[1] = SPSEL
CONTROL[0] = nPRIV
```

Thus:

``` text
0x2 = 0b10

SPSEL = 1
nPRIV = 0
```

Meaning:

``` text
Thread mode uses PSP
Thread mode remains privileged
```

This matches the standard non-MPU ARM_CM4F FreeRTOS port.

The measured:

``` text
sp  = 0x20000630
psp = 0x20000630
```

also confirms that the active `SP` alias resolves to PSP in the running
task.

------------------------------------------------------------------------

## 16. xPSR Validation

Measured:

``` text
xPSR = 0x01000000
```

Bit 24 is the Thumb-state bit.

Therefore:

``` text
xPSR.T = 1
```

The CPU is executing in Thumb state.

The exception number field is zero at the sampled point, consistent with
execution inside `task_b` in Thread mode.

------------------------------------------------------------------------

## 17. Interrupt Mask Validation

Measured:

``` text
primask = 0x0
basepri = 0x0
```

At the sampled point Task B was executing normal application code.

Therefore:

``` text
PRIMASK = 0
```

means configurable interrupts are globally enabled.

``` text
BASEPRI = 0
```

means no FreeRTOS priority-threshold mask is active at that instant.

SysTick remains able to interrupt the task.

During kernel critical paths, FreeRTOS can temporarily raise `BASEPRI`
to the configured `0x50` threshold.

------------------------------------------------------------------------

## 18. FreeRTOS Stack Fill Pattern

Several registers contained:

``` text
0xA5A5A5A5
```

Examples:

``` text
r4  = 0xa5a5a5a5
r5  = 0xa5a5a5a5
r6  = 0xa5a5a5a5
r8  = 0xa5a5a5a5
r9  = 0xa5a5a5a5
r10 = 0xa5a5a5a5
r11 = 0xa5a5a5a5
```

FreeRTOS fills task stacks with `0xA5`.

Conceptually:

``` text
Initial stack:

A5 A5 A5 A5 A5 A5 A5 A5 A5 A5


After execution:

A5 A5 A5 A5 A5  XX XX XX XX XX
|-------------|  |-------------|
 untouched          used
```

The remaining untouched fill pattern is used for stack high-water-mark
analysis.

Stack usage is planned as a future profiler metric.

------------------------------------------------------------------------

## 19. Day 03 Completion Checklist

``` text
[✓] Custom bare-metal linker script
[✓] Custom startup assembly
[✓] Custom vector table
[✓] .data initialization
[✓] .bss initialization
[✓] Explicit MSP stack reservation

[✓] DWT cycle counter
[✓] Basic cycle profiling primitive

[✓] Custom FreeRTOSConfig.h
[✓] FreeRTOS kernel compilation
[✓] ARM_CM4F port integration
[✓] heap_4 integration
[✓] Application-owned ucHeap
[✓] Dedicated .heap linker section
[✓] Minimal memcpy/memset implementation

[✓] malloc-failure hook
[✓] stack-overflow hook
[✓] system panic path

[✓] SVC handler routing
[✓] PendSV handler routing
[✓] SysTick handler routing

[✓] Scheduler starts on physical STM32F411
[✓] First task context restoration
[✓] Thread-mode PSP execution
[✓] MSP/PSP separation
[✓] SysTick-driven kernel tick
[✓] PendSV context switching
[✓] Equal-priority time slicing
[✓] Task stacks allocated from ucHeap
[✓] Live CPU state validated through GDB
```

------------------------------------------------------------------------

## 20. Current Architecture

``` text
CUSTOM BARE-METAL PLATFORM
          |
          v
custom linker script
          |
          v
custom startup and vector table
          |
          v
minimal freestanding C environment
          |
          v
DWT cycle counter
          |
          v
FreeRTOS generic kernel
          |
          v
ARM_CM4F port
          |
          v
heap_4 + application-owned ucHeap
          |
          v
preemptive scheduler
          |
          v
SysTick + PendSV context switching
          |
          v
Task A <------> Task B
```

The FreeRTOS platform bring-up phase is complete.

------------------------------------------------------------------------

## 21. Next Phase --- Kernel Profiler Architecture

The next session begins actual kernel instrumentation.

Scheduling path to inspect:

``` text
SysTick_Handler
        |
        v
xTaskIncrementTick()
        |
        v
PendSV requested
        |
        v
PendSV_Handler
        |
        v
vTaskSwitchContext()
        |
        v
pxCurrentTCB changes
```

Planned profiler metrics:

``` text
Outgoing task:
- elapsed execution cycles
- cumulative execution cycles
- switch-out count

Incoming task:
- switch-in cycle timestamp
- switch-in count

Per-task diagnostics:
- stack high-water mark
- task priority/state metadata

Global diagnostics:
- total context-switch count
- scheduler/runtime totals
```

The next architectural decision is the instrumentation boundary.

Candidate approaches:

``` text
1. FreeRTOS trace macros
   traceTASK_SWITCHED_IN
   traceTASK_SWITCHED_OUT

2. Direct scheduler/port modification
   tasks.c / port.c instrumentation
```

The decision will be made by tracing the exact cloned FreeRTOS
implementation and separating:

``` text
tasks.c   --> scheduler policy
port.c    --> Cortex-M context mechanism
profiler  --> measurement and accounting
```

------------------------------------------------------------------------

## End-of-Day Status

**Day 03 result: FreeRTOS is fully brought up on the custom bare-metal
STM32F411 platform and the scheduler has been validated on physical
hardware.**

The project has progressed from:

``` text
bare-metal cycle counter
```

to:

``` text
working preemptive RTOS execution platform
```

with verified task creation, dynamic kernel allocation, synthetic
initial task contexts, SVC first-task startup, PSP task execution, MSP
exception execution, SysTick ticks, PendSV context switching, and
equal-priority time slicing.

**Next session: trace the exact switch path in `port.c` and `tasks.c`,
choose the profiler instrumentation boundary, and begin per-task runtime
accounting.**