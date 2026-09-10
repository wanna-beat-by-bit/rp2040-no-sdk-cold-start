#include <stdint.h>
#include "registers.h"

#define DELAY_LOOPS_FASTER 150000u
#define DELAY_LOOPS_SLOWER 1500000u
#define DATA_CANARY 0xABCDu

static void delay_loops(volatile uint32_t n) {
    while (n) { n--; }
}

volatile uint32_t g_panic_reason;
volatile uint32_t zeroed;
volatile uint32_t initialized = DATA_CANARY;

extern uint32_t __vectors_start;

static void panic(uint32_t reason) {
    g_panic_reason = reason;
    for(;;);
}

int main(){
    __vectors_start = 0xE000ED08;

    REG(RESETS_BASE + APB_ATOMIC_CLR + RESETS_RESET) = (1u << RESETS_RESET_IO_BANK0_LSB)
                                                     | (1u << RESETS_RESET_PADS_BANK0_LSB);

    while( !(REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_IO_BANK0_LSB)) ){;}
    while( !(REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_PADS_BANK0_LSB)) ){;}

    volatile uint32_t *const gpio25_ctrl_addr = (volatile uint32_t *)(IO_BANK0_BASE + IO_BANK0_GPIO25_CTRL);
    uint32_t gpio25_ctrl = REG(gpio25_ctrl_addr);
    gpio25_ctrl &= ~IO_BANK0_GPIO_CTRL_FUNCSEL_MASK;
    gpio25_ctrl |= IO_BANK0_GPIO_CTRL_FUNCSEL_SIO;
    REG(gpio25_ctrl_addr) = gpio25_ctrl;

    uint32_t check = REG(gpio25_ctrl_addr) & IO_BANK0_GPIO_CTRL_FUNCSEL_MASK;
    if (check != IO_BANK0_GPIO_CTRL_FUNCSEL_SIO){ panic(106); }

    REG(SIO_BASE + SIO_GPIO_OE_SET) = (1u << GPIO25_BIT);

    uint32_t read_gpio_oe = REG(SIO_BASE + SIO_GPIO_OE);
    uint32_t gpio25_gpio_oe = read_gpio_oe & (1u << GPIO25_BIT);
    if( !gpio25_gpio_oe ) { panic(107); }

    for(;;){
        REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
        if (initialized == DATA_CANARY && zeroed == 0){
            delay_loops(DELAY_LOOPS_FASTER);
        }else{
            delay_loops(DELAY_LOOPS_SLOWER);
        }
    }
}
