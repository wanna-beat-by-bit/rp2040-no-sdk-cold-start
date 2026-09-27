.cpu cortex-m0plus
.syntax unified
.thumb

.equ XIP_SSI_BASE, 0x18000000
.equ XIP_SSI_BASE_CTRLR0, XIP_SSI_BASE + 0x00
.equ XIP_SSI_BASE_SSIENR, XIP_SSI_BASE + 0x08
.equ XIP_SSI_BASE_BAUDR, XIP_SSI_BASE + 0x14
.equ XIP_SSI_BASE_SPI_CTRLR0, XIP_SSI_BASE + 0xf4
.equ XIP_SSI_BASE_SPI_CTRLR0_TMOD_EEPROM_READ, 0x3
.equ VECTOR_TABLE_ADDRR, 0x10000100

.section .boot2, "ax"
.global _boot2_start
.type _boot2_start, %function
.thumb_func

_boot2_start:
    @ set tx mode to 0x3
    ldr r0, =XIP_SSI_BASE_CTRLR0
    ldr r1, [r0]
    movs r2, #3 
    lsls r2, #8 
    bics r1, r2 
    orrs r1, r2 
    str r1, [r0]

    @ end of configuration SSI, jump back to _start
    ldr r3, =VECTOR_TABLE_ADDRR
    ldr r0, [r3]
    msr msp, r0
    ldr r3, [r3, #4]
    bx r3