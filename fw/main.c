#include <stdint.h>

#define  REG(addr) (*(volatile uint32_t *)(addr))

#define RESETS_BASE  0x4000c000
#define RESET_BIT_IO_BANK0 5u
#define RESET_BIT_PADS_BANK0 8u
#define RESET_DONE_OFFSET 0x8

#define IO_BANK0_BASE 0x40014000
#define GPIO25_CTRL_OFFSET 0x0cc 
#define GPIO25_STATUS_OFFSET 0x0c8 

#define FUNCSEL_SIO 5
#define FUNCSEL_WIDTH 5u
#define FUNCSEL_LSB 0 
#define FUNCSEL_MASK (((1u << FUNCSEL_WIDTH) - 1u) << FUNCSEL_LSB)

int main(){ 
    REG(RESETS_BASE) &= ~(1u << RESET_BIT_IO_BANK0);
    REG(RESETS_BASE) &= ~(1u << RESET_BIT_PADS_BANK0);

    while( !(REG(RESETS_BASE + RESET_DONE_OFFSET) & (1u << RESET_BIT_IO_BANK0)) ){;} 
    while( !(REG(RESETS_BASE + RESET_DONE_OFFSET) & (1u << RESET_BIT_PADS_BANK0)) ){;} 

    uint32_t gpio25_ctrl = REG(IO_BANK0_BASE + GPIO25_CTRL_OFFSET);
    gpio25_ctrl &= ~FUNCSEL_MASK;
    gpio25_ctrl |= FUNCSEL_SIO;
    REG(IO_BANK0_BASE + GPIO25_CTRL_OFFSET) = gpio25_ctrl;

    uint32_t check = REG(IO_BANK0_BASE + GPIO25_CTRL_OFFSET) & FUNCSEL_MASK;
    if (check != FUNCSEL_SIO){ for(;;); }

    for(;;);
}