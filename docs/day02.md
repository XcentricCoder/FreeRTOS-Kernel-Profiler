# Day 02 --- Startup Validation and Cortex-M4 Boot Debugging

## Objective

Validate the custom STM32F411 startup code on real hardware and
understand the complete execution path from reset to `main()`.

The focus for Day 02 was not only to make the firmware boot, but to
inspect the startup process instruction-by-instruction using GDB.

------------------------------------------------------------------------

## 1. Vector Table Verification

The `.isr_vector` section was inspected using:

``` bash
arm-none-eabi-objdump -s -j .isr_vector build/firmware.elf
```

Observed vector table:

``` text
0x08000000  0x20020000  0x08000041  0x0800006f  0x0800006f
0x08000010  0x0800006f  0x0800006f  0x0800006f  0x00000000
0x08000020  0x00000000  0x00000000  0x00000000  0x0800006f
0x08000030  0x0800006f  0x00000000  0x0800006f  0x0800006f
```

Key observations:

-   Initial MSP = `0x20020000`
-   `Reset_Handler` vector = `0x08000041`
-   Exception handlers point to `0x0800006f`
-   Bit 0 of handler addresses is set, indicating Thumb state

Handler symbols were verified using:

``` bash
arm-none-eabi-readelf -s build/firmware.elf | grep Handler
```

The weak exception handlers correctly resolved to `Default_Handler`.

------------------------------------------------------------------------

## 2. ST-LINK and OpenOCD Bring-Up

Initial OpenOCD attempts failed with:

``` text
Error: init mode failed (unable to connect to the target)
```

ST-LINK was visible over USB, but initially could not communicate with
the STM32 target.

The ST-LINK firmware was upgraded using ST's `STLinkUpgrade.jar`.

After the upgrade:

``` text
version: V2J48S35
```

A second issue was identified from OpenOCD:

``` text
Target voltage: 0.001577
Error: target voltage may be too low for reliable debugging
```

The ST-LINK/STM32 board jumpers were not correctly placed. After
correcting the jumper configuration, LD3 turned on and the target
voltage became:

``` text
Target voltage: 3.255227
```

OpenOCD then successfully detected the Cortex-M4:

``` text
stm32f4x.cpu: hardware has 6 breakpoints, 4 watchpoints
starting gdb server for stm32f4x.cpu on 3333
Listening on port 3333 for gdb connections
```

------------------------------------------------------------------------

## 3. GDB Connection to Real Hardware

Since `arm-none-eabi-gdb` was unavailable through the Ubuntu package
repository, `gdb-multiarch` was used.

``` bash
gdb-multiarch build/firmware.elf
```

Connection to OpenOCD:

``` gdb
target extended-remote localhost:3333
```

Firmware was flashed using:

``` gdb
load
```

The programmed sections were:

``` text
.isr_vector
.text
.rodata
.data
```

The vector table was verified directly from STM32 Flash:

``` gdb
x/16wx 0x08000000
```

After:

``` gdb
monitor reset halt
```

the processor correctly entered:

``` text
PC  = 0x08000040 <Reset_Handler>
MSP = 0x20020000
```

This confirmed that the custom vector table and reset entry point were
working correctly on the STM32F411.

------------------------------------------------------------------------

## 4. `.data` Initialization Debugging

The startup code initializes `.data` by copying initialized values from
Flash to SRAM.

Initial registers:

``` text
r0 = 0x080000d0    -> Flash load address (_sidata)
r1 = 0x20000000    -> SRAM destination (_sdata)
r2 = 0x20000008    -> end of .data (_edata)
```

The copy loop:

``` asm
DataCopyLoop:
    ldr r3, [r0], #4
    str r3, [r1], #4
    cmp r1, r2
    bcc DataCopyLoop
```

### First iteration

``` asm
ldr r3, [r0], #4
```

Loaded:

``` text
r3 = 0x0000000a
```

and post-incremented:

``` text
r0: 0x080000d0 -> 0x080000d4
```

Then:

``` asm
str r3, [r1], #4
```

stored `10` at:

``` text
SRAM[0x20000000] = 10
```

and incremented:

``` text
r1: 0x20000000 -> 0x20000004
```

### Second iteration

The next Flash word was:

``` text
0x080000c0
```

This was the address of the constant `table`.

After the second store:

``` text
0x20000000 -> global = 10
0x20000004 -> p = 0x080000c0
```

At this point:

``` text
r1 = 0x20000008
r2 = 0x20000008
```

Therefore `bcc DataCopyLoop` was not taken and `.data` initialization
completed.

------------------------------------------------------------------------

## 5. `.bss` Zero Initialization

The `.bss` region was:

``` text
_sbss = 0x20000008
_ebss = 0x2000000c
```

Startup code:

``` asm
ZeroBSS:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

    cmp r1, r2
    beq ZeroBSSDone

ZeroBSSLoop:
    str r3, [r1], #4
    cmp r1, r2
    bcc ZeroBSSLoop
```

Registers before clearing:

``` text
r1 = 0x20000008
r2 = 0x2000000c
r3 = 0
```

The instruction:

``` asm
str r3, [r1], #4
```

stored zero at `0x20000008`.

This initialized:

``` c
int uninit;
```

to zero.

After the store:

``` text
r1 = 0x2000000c
r2 = 0x2000000c
```

The loop terminated.

------------------------------------------------------------------------

## 6. Transition from Startup Code to C

After `.data` and `.bss` initialization:

``` asm
bl main
```

was executed.

GDB confirmed entry into:

``` text
0x08000084 <main>
```

The first instruction generated by GCC was:

``` asm
push {r7}
```

This marked the transition from manually written startup assembly into
compiler-generated C function code.

------------------------------------------------------------------------

## 7. Generated Assembly for `main.c`

The C source was compiled into assembly to inspect GCC output.

The generated assembly showed the placement of global objects:

``` asm
.section .data.global,"aw"
global:
    .word 10
```

``` asm
.section .bss.uninit,"aw",%nobits
uninit:
    .space 4
```

``` asm
.section .rodata.table,"a"
table:
    .word 1
    .word 2
    .word 3
    .word 4
```

``` asm
.section .data.p,"aw"
p:
    .word table
```

This directly connected the C declarations to the linker sections
previously studied.

------------------------------------------------------------------------

## 8. Understanding the `main()` Stack Frame

GCC generated the following function prologue:

``` asm
push {r7}
sub sp, sp, #12
add r7, sp, #0
```

Initial MSP:

``` text
0x20020000
```

After:

``` asm
push {r7}
```

the stack pointer became:

``` text
SP = 0x2001fffc
```

The old `r7` value was physically stored on the stack.

After:

``` asm
sub sp, sp, #12
```

the stack pointer became:

``` text
SP = 0x2001fff0
```

This reserved 12 bytes of stack-frame space.

Then:

``` asm
add r7, sp, #0
```

made `r7` the frame pointer:

``` text
SP = 0x2001fff0
r7 = 0x2001fff0
```

Important distinction learned:

-   `sub sp, sp, #12` only reserves memory
-   it does not write values into that memory
-   `push`, `str`, and similar store instructions actually write data to
    the stack

------------------------------------------------------------------------

## 9. Local Variable Storage

For:

``` c
int local = 5;
```

GCC generated:

``` asm
movs r3, #5
str r3, [r7, #4]
```

Before the store:

``` text
r3 = 5
r7 = 0x2001fff0
```

Therefore:

``` text
r7 + 4 = 0x2001fff4
```

After the store:

``` text
SRAM[0x2001fff4] = 5
```

GDB verification:

``` gdb
x/wx 0x2001fff4
```

returned:

``` text
0x2001fff4: 0x00000005
```

and:

``` gdb
p local
```

returned:

``` text
$1 = 5
```

This demonstrated how a local C variable is physically represented
inside a stack frame.

------------------------------------------------------------------------

## 10. Debugging `global++`

For:

``` c
global++;
```

GCC generated a load-modify-store sequence.

First:

``` asm
ldr r3, .L3
```

loaded the address of `global`:

``` text
r3 = 0x20000000
```

Then:

``` asm
ldr r3, [r3]
```

dereferenced the address:

``` text
r3 = 10
```

Then:

``` asm
adds r3, r3, #1
```

produced:

``` text
r3 = 11
```

Finally the value was stored back to SRAM.

GDB confirmed:

``` gdb
x/wd 0x20000000
```

``` text
0x20000000 <global>: 11
```

This clarified the difference between:

``` text
address of a variable
```

and:

``` text
value stored at that address
```

------------------------------------------------------------------------

## 11. Debugging `uninit = local`

Execution reached:

``` c
uninit = local;
```

Relevant assembly:

``` asm
ldr r2, .L3+4
ldr r3, [r7, #4]
str r3, [r2, #0]
```

Observed registers:

``` text
r2 = 0x20000008    -> address of uninit
r3 = 5             -> value of local
```

The next instruction at the end of Day 02 was:

``` asm
str r3, [r2, #0]
```

This stores:

``` text
5 -> SRAM[0x20000008]
```

Therefore:

``` c
uninit = 5;
```

### Exact debugging checkpoint

``` text
PC = 0x0800009c
Instruction = str r3, [r2, #0]

SP = 0x2001fff0
r7 = 0x2001fff0
r3 = 5
r2 = 0x20000008
```

Day 03 can resume directly from the execution of this instruction.

------------------------------------------------------------------------

## Key Concepts Learned

-   Cortex-M4 vector table structure
-   Thumb bit in exception handler addresses
-   Weak exception handler aliases
-   ST-LINK firmware upgrade and target-voltage debugging
-   OpenOCD GDB server setup
-   Flashing a custom ELF to STM32F411
-   Hardware reset flow into `Reset_Handler`
-   `.data` LMA-to-VMA copy process
-   `.bss` zero initialization
-   Post-indexed ARM addressing
-   `cmp`, `bcc`, and `beq` in startup loops
-   Transition from startup assembly to C
-   GCC-generated Cortex-M4 assembly
-   Stack growth direction
-   Function prologue and stack-frame allocation
-   Difference between reserving stack space and writing stack data
-   Frame pointer usage with `r7`
-   Local variable storage on the stack
-   Address loading versus memory dereferencing
-   Load-modify-store implementation of `global++`

------------------------------------------------------------------------

## Day 02 Status

The custom STM32F411 startup path has now been validated on real
hardware.

Verified execution path:

``` text
Reset
  |
  v
Vector Table
  |
  v
Reset_Handler
  |
  +--> Copy .data: Flash -> SRAM
  |
  +--> Zero .bss
  |
  v
main()
  |
  +--> Create stack frame
  |
  +--> Initialize local variable
  |
  +--> global++
  |
  +--> uninit = local   <-- current GDB checkpoint
```

The project is now ready to continue through the remaining C execution
and then move from startup validation toward FreeRTOS kernel integration
and profiler development.