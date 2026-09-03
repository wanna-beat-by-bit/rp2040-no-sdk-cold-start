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
    REG(RESETS_BASE + APB_ATOMIC_CLR + RESET_OFFSET) = (1u << RESET_BIT_IO_BANK0)
                                                     | (1u << RESET_BIT_PADS_BANK0);

    while( !(REG(RESETS_BASE + RESET_DONE_OFFSET) & (1u << RESET_BIT_IO_BANK0)) ){;}
    while( !(REG(RESETS_BASE + RESET_DONE_OFFSET) & (1u << RESET_BIT_PADS_BANK0)) ){;}

    volatile uint32_t *const gpio25_ctrl_addr = (volatile uint32_t *)(IO_BANK0_BASE + GPIO25_CTRL_OFFSET);
    uint32_t gpio25_ctrl = REG(gpio25_ctrl_addr);
    gpio25_ctrl &= ~FUNCSEL_MASK;
    gpio25_ctrl |= FUNCSEL_SIO;
    REG(gpio25_ctrl_addr) = gpio25_ctrl;

    uint32_t check = REG(gpio25_ctrl_addr) & FUNCSEL_MASK;
    if (check != FUNCSEL_SIO){ panic(106); }

    REG(SIO_BASE + SIO_GPIO_OE_SET_OFFSET) = (1u << GPIO25_BIT);

    uint32_t read_gpio_oe = REG(SIO_BASE + SIO_GPIO_OE_OFFSET);
    uint32_t gpio25_gpio_oe = read_gpio_oe & (1u << GPIO25_BIT);
    if( !gpio25_gpio_oe ) { panic(107); }

    /* GPIO_OUT resets to 0, so the first XOR is the on-edge. One write per half-period
       replaces the set/clear pair — the toggle is what we mean, so say it directly. */
    for(;;){
        REG(SIO_BASE + SIO_GPIO_OUT_XOR_OFFSET) = (1u << GPIO25_BIT);
        delay_loops(DELAY_LOOPS);
    }
}
