.cpu cortex-m0plus
.syntax unified
.thumb

.section .text.entry, "ax"
.type _start, %function
.global _start
.thumb_func
_start:
    ldr r0, =__stack_top
    mov sp, r0
    bl main
trap: b trap
