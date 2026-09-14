#include <stdint.h>
#include "registers.h"

#define DELAY_FASTER 150000u
#define DELAY_SLOWER 1500000u
#define DATA_CANARY 0xABCDu

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

    alarm_set();

    for(;;);
}
