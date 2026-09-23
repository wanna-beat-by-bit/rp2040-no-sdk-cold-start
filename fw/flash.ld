
MEMORY
{
    FLASH (rwx) : ORIGIN = 0x10000000, LENGTH = 2M
    SRAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 264K
}

SECTIONS
{
    .boot2 : {
        __boot2_start__ = .;

        KEEP(*(.boot2))

        __boot2_end__ = .;
    } > FLASH

    .vectors : {
        . = ALIGN(256);
        __vectors_start__ = .;

        KEEP(*(.vectors))

        . = ALIGN(4);

        __vectors_end__ = .;
    } > SRAM

    .text : {
        . = ALIGN(4);

        KEEP(*(.text.entry))
        *(.text*)

        . = ALIGN(4);
    } > FLASH

    .rodata : {
        . = ALIGN(4);

        *(.rodata*)

        . = ALIGN(4);
    } > FLASH

    .data : { 
        . = ALIGN(4);
        __data_start__ = .;

        *(.data*)

        . = ALIGN(4);
        __data_end__ = .;
    } > SRAM AT > FLASH
    __load_addr__ = LOADADDR(.data);

    .bss : {
        . = ALIGN(4);
        
        __bss_start__ = .;

        *(.bss*)
        *(COMMON)

        . = ALIGN(4);

        __bss_end__ = .;
    } > FLASH

}