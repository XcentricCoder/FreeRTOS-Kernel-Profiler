# Day 03 --- DWT Cycle Counter Driver and Hardware Verification

## Objective

The goal for Day 03 was to build and verify the first hardware timing
primitive for the FreeRTOS Kernel Profiler: a cycle counter driver using
the Cortex-M4 Data Watchpoint and Trace (DWT) unit.

The driver provides:

-   initialization of the DWT cycle counter
-   access to the current `CYCCNT` value
-   cycle-accurate timestamp snapshots for future profiler measurements

## 1. DWT and CYCCNT Study

Relevant memory-mapped registers:

  Register              Address Purpose
  -------------- -------------- -------------------------------
  `DEMCR`          `0xE000EDFC` Debug and trace control
  `DWT_CTRL`       `0xE0001000` DWT control register
  `DWT_CYCCNT`     `0xE0001004` Current processor cycle count

Relevant fields:

-   `DEMCR.TRCENA` --- bit 24
-   `DWT_CTRL.NOCYCCNT` --- bit 25
-   `DWT_CTRL.CYCCNTENA` --- bit 0

Initialization sequence:

1.  Enable trace functionality using `DEMCR.TRCENA`.
2.  Check `DWT_CTRL.NOCYCCNT`.
3.  Reset `DWT_CYCCNT` to zero.
4.  Set `DWT_CTRL.CYCCNTENA`.

## 2. Cycle Counter Driver

Created the cycle counter module under `profiler/`.

### `profiler/cycle_counter.c`

``` c
#include "cycle_counter.h"
#include <stdint.h>

#define COREDEBUG_DEMCR        (*(volatile uint32_t *)0xE000EDFC)
#define DWT_CTRL               (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT             (*(volatile uint32_t *)0xE0001004)

#define COREDEBUG_DEMCR_TRCENA (1UL << 24)
#define DWT_CTRL_CYCCNTENA     (1UL << 0)
#define DWT_CTRL_NOCYCCNT      (1UL << 25)

void cycle_counter_init(void)
{
    COREDEBUG_DEMCR |= COREDEBUG_DEMCR_TRCENA;

    if (DWT_CTRL & DWT_CTRL_NOCYCCNT)
    {
        return;
    }

    DWT_CYCCNT = 0;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA;
}

uint32_t cycle_counter_get(void)
{
    return DWT_CYCCNT;
}
```

`cycle_counter_get()` returns a snapshot of the current counter. It does
not stop the counter.

## 3. Makefile Integration

Added:

``` makefile
C_SOURCES = core/main.c \
            profiler/cycle_counter.c

OBJECTS = \
        build/main.o \
        build/cycle_counter.o \
        build/startup_stm32f411.o

CFLAGS += -Iprofiler
```

Compile rule:

``` makefile
build/cycle_counter.o: profiler/cycle_counter.c | build
    $(CC) $(CFLAGS) -c $< -o $@
```

The `-Iprofiler` flag adds `profiler/` to the compiler header search
path.

## 4. Multiple Definition Linker Error

The linker initially reported:

``` text
multiple definition of `cycle_counter_init'
multiple definition of `cycle_counter_get'
```

The implementation source had accidentally been included into another
translation unit.

Correct structure:

``` c
#include "cycle_counter.h"
```

The `.c` implementation is compiled separately. Including a `.c` file
can place the same global function definitions in multiple object files
and cause duplicate linker symbols.

## 5. Test Program

``` c
#include <stdint.h>
#include "cycle_counter.h"

volatile uint32_t start;
volatile uint32_t stop;
volatile uint32_t elapsed;

int main(void)
{
    cycle_counter_init();

    start = cycle_counter_get();

    for (volatile uint32_t i = 0; i < 1000; i++)
    {
    }

    stop = cycle_counter_get();
    elapsed = stop - start;

    while (1)
    {
    }
}
```

The loop variable was made `volatile` so the empty test loop remains
observable and is not simply optimized away.

## 6. Build Verification

Successful build:

``` text
text    data    bss    dec    hex
300       0      12    312    138
```

The 12-byte `.bss` corresponds to three uninitialized `uint32_t`
globals:

``` text
start   = 4 bytes
stop    = 4 bytes
elapsed = 4 bytes
total   = 12 bytes
```

## 7. Register-Level GDB Verification

### DEMCR trace enable

The operation:

``` c
COREDEBUG_DEMCR |= COREDEBUG_DEMCR_TRCENA;
```

was observed as a read-modify-write sequence:

``` text
read DEMCR
    ↓
OR with 0x01000000
    ↓
write DEMCR
```

Final value:

``` text
DEMCR = 0x01000000
TRCENA = 1
```

### NOCYCCNT capability check

`DWT_CTRL` read as:

``` text
0x40000000
```

The `0x40000000` value was already hardware-reported information from
the DWT implementation. The upper `NUMCOMP` field reports four
comparator units.

Writing zero to a memory-mapped hardware register does not imply that
read-only or implementation-defined fields read back as zero.

The capability test compiled into:

``` asm
and.w r3, r3, #0x02000000
cmp   r3, #0
bne   ...
```

Result:

``` text
0x40000000 & 0x02000000 = 0
NOCYCCNT = 0
```

Therefore `CYCCNT` is implemented.

### Resetting CYCCNT

The statement:

``` c
DWT_CYCCNT = 0;
```

was observed as:

``` text
r3 = 0xE0001004
r2 = 0
str r2, [r3]
```

### Enabling CYCCNT

Before:

``` text
DWT_CTRL = 0x40000000
```

After setting bit 0:

``` text
DWT_CTRL = 0x40000001
```

Therefore:

``` text
CYCCNTENA = 1
```

The hardware cycle counter was successfully enabled.

## 8. CYCCNT Behaviour Under GDB

While the core was halted, repeated reads showed:

``` text
CYCCNT = 2
CYCCNT = 2
```

The core was halted, so normal processor execution and cycle counting
were not progressing.

After `continue`, the core resumed and `CYCCNT` increased.

After `Ctrl+C`, the core halted again and the observed `CYCCNT` value
remained fixed.

## 9. End-to-End Cycle Measurement

Measured values:

``` text
start   = 14
stop    = 10042
elapsed = 10028
```

Verification:

``` text
10042 - 14 = 10028
```

After the program continued in the final infinite loop, the current
counter eventually reached:

``` text
CYCCNT = 65951507
```

This does not invalidate the measurement.

The timeline is:

``` text
CYCCNT:
0 → ... → 14 → ... → 10042 → ... → 65951507
          ↑             ↑                ↑
       start          stop          GDB halt
```

`start` and `stop` are snapshots of a continuously running hardware
counter.

The measured region was:

``` text
start snapshot
      ↓
1000-iteration loop
      ↓
stop snapshot
```

Therefore:

``` text
elapsed = stop - start
        = 10028 cycles
```

The later `CYCCNT` value is simply the current counter state after the
CPU continued executing the infinite branch loop for a much longer
duration.

## Key Concepts Learned

-   DWT provides hardware cycle-accurate timing.
-   `CYCCNT` is a continuously running 32-bit hardware counter when
    enabled.
-   `cycle_counter_get()` returns a snapshot and does not stop the
    counter.
-   Memory-mapped hardware registers do not behave exactly like RAM.
-   Read-only and implementation fields can remain set after writing
    zero.
-   C `|=` register operations compile into read-modify-write sequences.
-   `x/wx` displays a 32-bit word in hexadecimal.
-   `x/wu` displays the same word as unsigned decimal.
-   A halted processor leaves the observed cycle counter fixed.
-   Execution time can be measured using two counter snapshots and
    subtraction.

## Day 03 Milestone

``` text
DWT initialization      ✅
CYCCNT capability check ✅
Cycle counter reset     ✅
Cycle counter enable    ✅
Cycle snapshot API      ✅
GDB register validation ✅
End-to-end measurement  ✅
```

The first hardware timing primitive of the FreeRTOS Kernel Profiler is
complete and verified on the STM32F411.

## Next Step

Build the profiler core abstraction on top of the cycle counter.

The profiler core will track:

``` text
start cycle
last elapsed cycles
total accumulated cycles
execution/run count
maximum execution cycles
```

Later, the profiler will be connected to FreeRTOS context-switch events
so task runtime can be accumulated automatically.