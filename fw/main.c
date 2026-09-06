#include <stdint.h>
#include "registers.h"

#define DELAY_LOOPS 500000u

static void delay_loops(volatile uint32_t n) {
    while (n) { n--; }
}

volatile uint32_t g_panic_reason;

static void panic(uint32_t reason) {
    g_panic_reason = reason;
    for(;;);
}

int main(){
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

    /* GPIO_OUT resets to 0, so the first XOR is the on-edge. One write per half-period
       replaces the set/clear pair — the toggle is what we mean, so say it directly. */
    for(;;){
        REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
        delay_loops(DELAY_LOOPS);
    }
}
