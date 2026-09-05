
#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

#define  REG(addr) (*(volatile uint32_t *)(addr))

#define APB_ATOMIC_XOR 0x1000u
#define APB_ATOMIC_SET 0x2000u
#define APB_ATOMIC_CLR 0x3000u

#define RESETS_BASE 0x4000c000u
#define RESET_BIT_IO_BANK0 5u
#define RESET_BIT_PADS_BANK0 8u
#define RESET_OFFSET 0x0u
#define RESET_DONE_OFFSET 0x8u

#define IO_BANK0_BASE 0x40014000u
#define GPIO25_CTRL_OFFSET 0x0ccu

#define FUNCSEL_SIO 5u
#define FUNCSEL_WIDTH 5u
#define FUNCSEL_LSB 0u
#define FUNCSEL_MASK (((1u << FUNCSEL_WIDTH) - 1u) << FUNCSEL_LSB)

#define SIO_BASE 0xd0000000u
#define SIO_GPIO_OUT_XOR_OFFSET 0x01cu
#define SIO_GPIO_OE_OFFSET 0x020u
#define SIO_GPIO_OE_SET_OFFSET 0x024u
#define GPIO25_BIT 25u


#endif 