ENTRY(Reset_Handler)

MEMORY 
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 512K 
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
}

/* Initial Main Stack Pointer */

_Min_Stack_Size = 0x800;
_estack = ORIGIN(RAM)+ LENGTH(RAM);
_sstack = _estack - _Min_Stack_Size;


SECTIONS 
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);

        *(.text)
        *(.text*)

        . = ALIGN(4);

    } > FLASH

    .rodata :
    {
        . = ALIGN(4);
        
        *(.rodata)
        *(.rodata*)

        . = ALIGN(4);

    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;

        *(.data)
        *(.data*)

        . = ALIGN(4);
        _edata = .;

    } > RAM AT > FLASH

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;

        *(.bss)
        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        _ebss = .;

    } > RAM 

    .heap(NOLOAD) :
    {
        . = ALIGN(8);
        _sheap = .;

        *(.heap)
        *(.heap*)

        . = ALIGN(8);
        _eheap = .;

    } > RAM


}

ASSERT(_eheap <= _sstack, "RAM overflow: heap overlaps MSP stack")