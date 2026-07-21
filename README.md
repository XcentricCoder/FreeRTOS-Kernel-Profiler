# FreeRTOS Kernel Runtime Profiler

A bare-metal FreeRTOS kernel runtime profiler implemented on the
STM32F411RE (Cortex-M4), built without STM32CubeMX, HAL, or
vendor-generated startup code.

The project brings up FreeRTOS from a minimal bare-metal environment and
instruments scheduler context switches to measure per-task CPU runtime
using the ARM Cortex-M4 DWT cycle counter.

## Features

-   Bare-metal STM32F411RE firmware bring-up
-   Custom ARM Cortex-M4 startup assembly and interrupt vector table
-   Custom GNU linker script for Flash and SRAM layout
-   FreeRTOS kernel integration without STM32 HAL or Cube-generated code
-   DWT `CYCCNT` based cycle-accurate runtime measurement
-   Automatic task discovery through FreeRTOS context-switch trace hooks
-   64-bit accumulated runtime accounting per task
-   Per-task context-switch counting
-   Per-task CPU utilization calculation
-   Task-name lookup through FreeRTOS task handles
-   Consistent snapshot API for retrieving profiler statistics
-   Hardware validation using OpenOCD and GDB

## Target

### Hardware

-   STM32 Nucleo-F411RE
-   STM32F411RE MCU
-   ARM Cortex-M4F

### Toolchain

-   `arm-none-eabi-gcc`
-   GNU Make
-   OpenOCD
-   GDB / `gdb-multiarch`

No STM32CubeMX-generated initialization code or STM32 HAL is required.

## Architecture

The profiler observes FreeRTOS scheduler context switches rather than
requiring manual instrumentation inside individual tasks.

``` text
FreeRTOS Scheduler
        |
Context Switch Events
        |
traceTASK_SWITCHED_IN / OUT
        |
Profiler Trace Hooks
        |
Task Profiler Core
        |
ARM DWT CYCCNT
        |
Per-Task Runtime Statistics
```

When a task is switched in, the profiler records the current value of
`DWT->CYCCNT`. When it is switched out, elapsed cycles are accumulated
into a 64-bit per-task runtime counter.

``` text
elapsed_cycles = switch_out_cycle - switch_in_cycle
task_total_cycles += elapsed_cycles
```

Unsigned subtraction handles a single wrap of the 32-bit `CYCCNT`
counter. The 64-bit accumulated runtime supports profiling over much
longer periods.

## CPU Utilization

CPU utilization is calculated from accumulated task runtimes:

``` text
task_cpu_usage = (task_runtime_cycles / total_runtime_cycles) * 100
```

The API stores utilization in units of `0.01%`:

``` text
9948 -> 99.48%
  51 ->  0.51%
```

The profiler also accounts for the live elapsed interval of the
currently executing task when generating runtime statistics.

## Snapshot API

``` c
typedef struct
{
    TaskHandle_t task;
    const char *name;
    uint64_t runtime_cycles;
    uint32_t switch_count;
    uint16_t cpu_usage;
} task_profiler_snapshot_t;
```

A coherent per-task snapshot is obtained with:

``` c
BaseType_t task_profiler_get_snapshot(
    uint32_t index,
    task_profiler_snapshot_t *snapshot
);
```

The profiler also exposes lightweight query functions for task count,
task name, runtime cycles, total runtime, and CPU utilization. Internal
task-registration and context-switch accounting functions remain private
to the implementation.

## Hardware Validation

The profiler was validated on STM32F411RE hardware using two
equal-priority FreeRTOS tasks with deliberately different execution
behavior.

### Test Workload

`TaskA` is CPU-bound:

``` c
for(;;)
{
    task_a_counter++;
}
```

`TaskB` performs a small amount of work and then blocks:

``` c
for(;;)
{
    task_b_counter++;
    vTaskDelay(pdMS_TO_TICKS(10U));
}
```

### Measured Result

| Task  | Runtime Cycles | Switch Count | CPU Usage |
| ----- | -------------: | -----------: | --------: |
| TaskA |    931,019,847 |        5,888 |    99.48% |
| TaskB |      4,815,562 |        5,887 |     0.51% |


The near-identical switch counts but dramatically different accumulated
runtime demonstrate that the profiler measures actual CPU execution time
rather than simply counting scheduler activations.

The FreeRTOS Timer Service task was also automatically discovered,
resulting in three registered tasks during the test. Earlier validation
with two continuously runnable equal-priority workloads produced an
approximately balanced CPU-runtime distribution.

## Project Structure

``` text
.
├── config/
│   └── FreeRTOSConfig.h
├── core/
│   ├── freertos_heap.c
│   ├── hooks.c
│   ├── main.c
│   ├── memory.c
│   ├── system_panic.c
│   └── system_panic.h
├── docs/
│   ├── day01.md
│   ├── day02.md
│   ├── day03.md
│   └── day04.md
├── linker/
│   └── stm32f411.ld
├── profiler/
│   ├── cycle_counter.c
│   ├── cycle_counter.h
│   ├── profiler_hooks.h
│   ├── task_profiler.c
│   └── task_profiler.h
├── startup/
│   └── startup_stm32f411.s
├── freertos/
│   └── FreeRTOS-Kernel/
├── Makefile
└── README.md
```

## Build

``` bash
make
```

For a clean rebuild:

``` bash
make clean
make
```

The build generates `firmware.elf`, `firmware.bin`, `firmware.hex`, and
`firmware.map` under `build/`.

## Debugging

Start OpenOCD:

``` bash
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg
```

Then start GDB:

``` bash
gdb-multiarch build/firmware.elf
```

Connect and load:

``` gdb
target extended-remote localhost:3333
monitor reset halt
load
monitor reset init
continue
```

Profiler state and task behavior can then be inspected directly through
GDB.

## Implementation Scope

Implemented as part of this project:

-   Cortex-M4 startup and reset flow
-   Interrupt vector integration
-   Flash/SRAM linker layout
-   `.data` initialization and `.bss` clearing
-   Bare-metal build and linking flow
-   FreeRTOS configuration and board integration
-   DWT cycle-counter abstraction
-   FreeRTOS trace-hook integration
-   Per-task runtime accounting
-   Context-switch counting
-   CPU utilization calculation
-   Profiler query and snapshot APIs
-   Hardware-level validation and debugging

The FreeRTOS scheduler and kernel primitives themselves are provided by
the upstream FreeRTOS Kernel source.

## Design Goals

The profiler is designed to remain:

-   Lightweight
-   Scheduler-driven
-   Independent of task-level manual instrumentation
-   Suitable for bare-metal FreeRTOS systems
-   Simple to inspect using standard embedded debugging tools

## Status

**V1: Core profiler complete and hardware validated.**

The current implementation provides automatic per-task runtime
profiling, context-switch statistics, CPU utilization, task
identification, and a consistent snapshot interface on STM32F411RE
hardware.