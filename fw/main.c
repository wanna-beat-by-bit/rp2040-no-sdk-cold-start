#include <stdint.h>
#include "registers.h"

#define SECOND_DELAY 500000u
#define DELAY_FASTER 150000u
#define DELAY_SLOWER 1500000u
#define DATA_CANARY 0xABCDu
#define DEFUALT_CLOCK_KHZ 12000
#define DEFUALT_CLOCK_MHZ 12
#define CLOCKS_FC0_INTERVAL_RESET 0x08u

volatile uint32_t g_panic_reason;
volatile uint32_t zeroed;
volatile uint32_t initialized = DATA_CANARY;

extern uint32_t __vectors_start;

static void panic(uint32_t reason) {
    g_panic_reason = reason;
    for(;;);
}

// alarm_set arms alarm to hardcoded duration.
static void alarm_set(void){
    uint32_t current_time = REG(TIMER_BASE + TIMER_TIMERAWL);
    REG(TIMER_BASE + TIMER_ALARM0) = current_time + SECOND_DELAY;
}

static void spin(volatile uint32_t n) { while (n) { n--; } }

static void configure_clock(void) {
    REG(CLOCKS_BASE + CLOCKS_FC0_REF_KHZ) = DEFUALT_CLOCK_KHZ; 
    REG(CLOCKS_BASE + CLOCKS_FC0_MIN_KHZ) = 0;
    REG(CLOCKS_BASE + CLOCKS_FC0_MAX_KHZ) = 0xFFFFFFFF;
    REG(CLOCKS_BASE + CLOCKS_FC0_INTERVAL) = CLOCKS_FC0_INTERVAL_RESET;
}

static void set_clk_ref_xosc(void) {
    uint32_t clk_ref_ctrl = REG(CLOCKS_BASE + CLOCKS_CLK_REF_CTRL);
    clk_ref_ctrl &= ~CLOCKS_CLK_REF_CTRL_SRC_MASK;
    clk_ref_ctrl |= CLOCKS_CLK_REF_CTRL_XOSC_CLKSRC;
    REG(CLOCKS_BASE + CLOCKS_CLK_REF_CTRL) = clk_ref_ctrl;
}

static void blink(void){
    REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
    spin(90000);
    REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
    spin(90000);
}

static void blink2(void){
    REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
}

void TIMER_IRQ_0_Handler(void){
    REG(TIMER_BASE + TIMER_INTR) = (1u << TIMER_INTR_ALARM_0); // w1c
    blink2();
    alarm_set();
}

static void set_wait_watchdog_tick(void) {
    uint32_t watchdog_reg = REG(WATCHDOG_BASE + WATCHDOG_TICK);
    watchdog_reg &= ~((1u << (WATCHDOG_TICK_CYCLES_MSB + 1)) - 1);
    watchdog_reg |= DEFUALT_CLOCK_MHZ;
    watchdog_reg |= 1u << WATCHDOG_TICK_ENABLE;
    REG(WATCHDOG_BASE + WATCHDOG_TICK) = watchdog_reg;
    while( !(REG(WATCHDOG_BASE + WATCHDOG_TICK) & (1u << WATCHDOG_TICK_RUNNING))) {;}
}


void HardFault_Handler(void){
    for (;;){
        blink();
        blink();
        blink();
        spin(1000000);
    }
}

static uint32_t is_xosc_stable(void) {
    uint32_t xosc_info = REG(XOSC_BASE + XOSC_STATUS);
    return (xosc_info & (1u << XOSC_STATUS_STABLE)) ? 1: 0;
}

static void configure_xosc(void) {
    // set xosc freq range
    REG(XOSC_BASE + XOSC_STARTUP) = XOSC_STARTUP_DELAY; 
    // enable and turn freq mode
    REG(XOSC_BASE + XOSC_CTRL) = (XOSC_CTRL_ENABLE << XOSC_CTRL_ENABLE_LSB) | XOSC_CTRL_1_15_RANGE;
    while (!(is_xosc_stable())) {;}
}

static void  wait_clk_ref_selected(void){
    while (!(REG(CLOCKS_BASE + CLOCKS_CLK_REF_SELECTED) & (1u << CLOCKS_CLK_REF_CTRL_XOSC_CLKSRC))) {;}

}

int main(){
    REG(PPB_BASE + SCB_VTOR) = (uint32_t)&__vectors_start;
    REG(PPB_BASE + NVIC_ISER) = (1u << NVIC_ISER_TIMER_IRQ_0);

    REG(RESETS_BASE + APB_ATOMIC_CLR + RESETS_RESET) = (1u << RESETS_RESET_IO_BANK0_LSB)
                                                     | (1u << RESETS_RESET_PADS_BANK0_LSB)
                                                     | (1u << RESETS_RESET_TIMER);

    while( !(REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_IO_BANK0_LSB)) ){;}
    while( !(REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_PADS_BANK0_LSB)) ){;}
    while( !( REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_TIMER) )) {;}

    // allow CPU process our IRQ with our implemented TIMER_IRQ_0 handler 
    REG(TIMER_BASE + TIMER_INTE) |= (1u << TIMER_INTE_ALARM_0); 

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

    // turn off alarm for now
    configure_clock();
    configure_xosc();

    set_clk_ref_xosc();
    wait_clk_ref_selected();

    set_wait_watchdog_tick();
    alarm_set();

    for(;;);
}
