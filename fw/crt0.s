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
    bhs .L_bss_loop_end
    str r2, [r0]
    adds r0, #4
    b .L_bss_loop


.L_bss_loop_end:

    ldr r0, =__data_start__
    ldr r1, =__data_end__
    ldr r2, =__data_load__

.L_data_loop:
    cmp r0,r1
    bhs .L_data_loop_end
    ldr r3, [r2]
    str r3, [r0]
    adds r0, #4
    adds r2, #4
    b .L_data_loop

.L_data_loop_end:
    bl main
.L_trap: b .L_trap
