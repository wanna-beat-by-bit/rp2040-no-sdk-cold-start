#include <stdint.h>

#define  REG(addr) (*(volatile uint32_t *)(addr))

#define RESETS_BASE  0x4000c000
#define RESET_IO_BANK0_BASE 5u
#define RESET_PADS_BANK0_BASE 8u

#define RESET_DONE 0x8


int main(){ 
    REG(RESETS_BASE) &= ~(1u << RESET_IO_BANK0_BASE);
    REG(RESETS_BASE) &= ~(1u << RESET_PADS_BANK0_BASE);

    while( !(REG(RESETS_BASE + RESET_DONE) & (1u << RESET_IO_BANK0_BASE)) ){;} 
    while( !(REG(RESETS_BASE + RESET_DONE) & (1u << RESET_PADS_BANK0_BASE)) ){;} 

    for(;;);
}