.syntax unified
.cpu cortex-m4
.thumb

.global Reset_Handler

.extern _estack

.extern main
.extern _sidata
.extern _sdata
.extern _edata
.extern _sbss
.extern _ebss

.section .isr_vector, "a", %progbits
.align 2

.word _estack
.word Reset_Handler
.word NMI_Handler
.word HardFault_Handler
.word MemManage_Handler
.word BusFault_Handler
.word UsageFault_Handler

.word 0
.word 0
.word 0
.word 0

.word SVC_Handler
.word DebugMon_Handler

.word 0

.word PendSV_Handler
.word SysTick_Handler

.section .text
.align 2

.type Reset_Handler, %function
.thumb_func
Reset_Handler:
DataCopy:
    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata

    cmp r1,r2
    beq DataCopyDone

    DataCopyLoop:
        ldr r3,[r0],#4
        str r3,[r1],#4
        cmp r1,r2
        bcc DataCopyLoop

    DataCopyDone:

ZeroBSS:
    ldr r1, =_sbss
    ldr r2, =_ebss
    movs r3, #0

    cmp r1,r2
    beq ZeroBSSDone

    ZeroBSSLoop:
        str r3,[r1],#4
        cmp r1,r2
        bcc ZeroBSSLoop

    ZeroBSSDone:
        bl main

    ForeverLoop:
        b ForeverLoop

.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler

.weak MemManage_Handler
.thumb_set MemManage_Handler, Default_Handler

.weak BusFault_Handler
.thumb_set BusFault_Handler, Default_Handler

.weak UsageFault_Handler
.thumb_set UsageFault_Handler, Default_Handler

.weak SVC_Handler
.thumb_set SVC_Handler, Default_Handler

.weak DebugMon_Handler
.thumb_set DebugMon_Handler, Default_Handler

.weak PendSV_Handler
.thumb_set PendSV_Handler, Default_Handler

.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler

.type Default_Handler, %function
.thumb_func
Default_Handler:
    b .

