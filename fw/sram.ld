ENTRY(_start)
 
MEMORY
{
    FLASH (rx) : ORIGIN = 0x10000000, LENGTH = 2M
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 264K
}

SECTIONS
{
    .text :
    {
        . = ALIGN(4);

        KEEP(*(.text.entry))
        *(.text*)

        . = ALIGN(4);
    } > SRAM

    .rodata :
    {
        . = ALIGN(4);
        *(.rodata*)
        . = ALIGN(4);
    } > SRAM

    .data :
    {
        . = ALIGN(4);
        __data_start__ = .;
        
        *(.data*)

        . = ALIGN(4);
        __data_end__ = .;
    } > SRAM                /* CP7: becomes  > SRAM AT > FLASH */
    __data_load__ = LOADADDR(.data);

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;

        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
        __bss_end__ = .;
    } > SRAM
}

__stack_top = ORIGIN(SRAM) + LENGTH(SRAM);