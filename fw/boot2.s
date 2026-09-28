.cpu cortex-m0plus
.syntax unified
.thumb

.equ XIP_SSI_BASE, 0x18000000
.equ XIP_SSI_BASE_CTRLR0, XIP_SSI_BASE + 0x00
.equ XIP_SSI_BASE_SSIENR, XIP_SSI_BASE + 0x08
.equ XIP_SSI_BASE_BAUDR, XIP_SSI_BASE + 0x14
.equ XIP_SSI_BASE_SPI_CTRLR0, XIP_SSI_BASE + 0xf4
.equ VECTOR_TABLE_ADDR, 0x10000100

.equ BAUDR_VALUE, 4
.equ CTRLR0_VALUE, (31 << 16) | (3 << 8)
.equ SPI_CTRLR0_VALUE, (0x03 << 24) | (2 << 8) | (6 << 2)

.section .boot2, "ax"
.global _boot2_entry
.type _boot2_entry, %function
.thumb_func

_boot2_entry:
    @ disable ssi before configuring
    ldr r0, =XIP_SSI_BASE_SSIENR
    movs r1, #0
    str r1, [r0]

    @ configure BAUDR
    ldr r0, =XIP_SSI_BASE_BAUDR
    movs r1, #BAUDR_VALUE
    str r1, [r0]

    @ configure CTRLR0
    @ TMOD to 0x3 mode, SPI_FRF standard, DFS_32 to 31
    ldr r0, =XIP_SSI_BASE_CTRLR0
    ldr r1, =CTRLR0_VALUE
    str r1, [r0]

    @ configure SPI_CTRLR0
    @ XIP_CMD to 0x03, INST_L to 8 bit, ADDR_L to 24 bit, TRANS_TYPE to 0
    ldr r0, =XIP_SSI_BASE_SPI_CTRLR0
    ldr r1, =SPI_CTRLR0_VALUE
    str r1, [r0]

    @ set SSIENR
    ldr r0, =XIP_SSI_BASE_SSIENR
    movs r1, #1
    str r1, [r0]

    @ end of configuration SSI
    @ 1. set up SP
    @ 2. jump to _start
    ldr r3, =VECTOR_TABLE_ADDR
    ldr r0, [r3]
    msr msp, r0
    ldr r3, [r3, #4]
    bx r3
