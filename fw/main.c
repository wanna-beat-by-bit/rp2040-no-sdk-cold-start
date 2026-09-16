#include <stdint.h>
#include "registers.h"

#define DELAY_FASTER 150000u
#define DELAY_SLOWER 1500000u
#define DATA_CANARY 0xABCDu
#define DEFUALT_CLOCK_KHZ 12000
#define CLOCKS_FC0_INTERVAL_RESET 0x08u
#define CLK_SYS_KHZ 12000

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
    REG(TIMER_BASE + TIMER_ALARM0) = current_time + DELAY_FASTER;
}

static void spin(volatile uint32_t n) { while (n) { n--; } }

static void configure_clock(void) {
    REG(CLOCKS_BASE + CLOCKS_FC0_REF_KHZ) = DEFUALT_CLOCK_KHZ; 
    REG(CLOCKS_BASE + CLOCKS_FC0_MIN_KHZ) = 0;
    REG(CLOCKS_BASE + CLOCKS_FC0_MAX_KHZ) = 0xFFFFFFFF;
    REG(CLOCKS_BASE + CLOCKS_FC0_INTERVAL) = CLOCKS_FC0_INTERVAL_RESET;
}

static int is_clk_valid(void) {
    REG(CLOCKS_BASE + CLOCKS_FC0_SRC) = CLOCKS_FC0_SRC_CLK_SYS;
    while( !(REG(CLOCKS_BASE + CLOCKS_FC0_STATUS) & (1u << CLOCKS_FC0_STATUS_DONE))) {;}
    uint32_t result = REG(CLOCKS_BASE + CLOCKS_FC0_RESULT);
    result = result >> 5;
    int is_valid =  ((result >= CLK_SYS_KHZ - 120) && (result <= CLK_SYS_KHZ + 120)) ? 1: 0;
    return is_valid;
}

void TIMER_IRQ_0_Handler(void){
    REG(TIMER_BASE + TIMER_INTR) = (1u << TIMER_INTR_ALARM_0); // w1c
    REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
    alarm_set();
}

static void blink(void){
    REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
    spin(40000);
    REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
    spin(40000);
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
    // alarm_set();
    configure_clock();
    configure_xosc();

    for(;;){
        if(is_xosc_stable()){
            blink();
            blink();
            spin(340000);
        } else{
            blink();
            spin(100000);
        }
    }
}
