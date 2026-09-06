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

    ldr r0, =__bss_start__
    ldr r1, =__bss_end__
    movs r2, #0

.L_bss_loop:    
    cmp r0,r1
    bhs .L_bss_end
    str r2, [r0]
    adds r0, #4
    b .L_bss_loop

.L_bss_end:
    bl main
trap: b trap
