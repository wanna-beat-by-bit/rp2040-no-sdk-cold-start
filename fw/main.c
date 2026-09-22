#include <stdint.h>
#include "registers.h"

#define SECOND_DELAY 500000u
#define DELAY_FASTER 150000u
#define DELAY_SLOWER 1500000u
#define DATA_CANARY 0xABCDu
#define DEFUALT_CLOCK_KHZ 12000
#define DEFUALT_CLOCK_MHZ 12
#define CLOCKS_FC0_INTERVAL_RESET 0x08u

/* loop counts, calibrated for a 125 MHz core - roughly 10x the 12 MHz values */
#define BLINK_SPIN 900000u
#define GAP_SPIN 10000000u

volatile uint32_t g_panic_reason;
volatile uint32_t zeroed;
volatile uint32_t initialized = DATA_CANARY;

extern uint32_t __vectors_start;

static uint32_t is_freq_in_range(uint32_t target_khz, uint32_t measured_khz, uint32_t tolerance_percent) {
      uint32_t margin = target_khz / 100u * tolerance_percent;
      uint32_t diff = (measured_khz > target_khz) ? (measured_khz - target_khz)
                                                  : (target_khz - measured_khz);
      return (diff <= margin) ? 1u : 0u;
  }

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
    spin(BLINK_SPIN);
    REG(SIO_BASE + SIO_GPIO_OUT_XOR) = (1u << GPIO25_BIT);
    spin(BLINK_SPIN);
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

static void pll_sys_init(void) {
    uint32_t pll_cs = REG(PLL_SYS_BASE + PLL_CS);
    pll_cs &= ~PLL_CS_REFDIV_MASK;
    pll_cs |= PLL_SYS_REFDIV << PLL_CS_REFDIV_LSB;
    REG(PLL_SYS_BASE + PLL_CS) = pll_cs;

    REG(PLL_SYS_BASE + PLL_FBDIV_INT) = PLL_SYS_FBDIV;

    REG(PLL_SYS_BASE + PLL_PWR) &= ~((1u << PLL_PWR_PD_BIT) | (1u << PLL_PWR_VCOPD_BIT));

    while (!(REG(PLL_SYS_BASE + PLL_CS) & (1u << PLL_CS_LOCK_BIT))) {;}

    uint32_t pll_prim = REG(PLL_SYS_BASE + PLL_PRIM);
    pll_prim &= ~(PLL_PRIM_POSTDIV1_MASK | PLL_PRIM_POSTDIV2_MASK);
    pll_prim |= (PLL_SYS_POSTDIV1 << PLL_PRIM_POSTDIV1_LSB)
              | (PLL_SYS_POSTDIV2 << PLL_PRIM_POSTDIV2_LSB);
    REG(PLL_SYS_BASE + PLL_PRIM) = pll_prim;

    REG(PLL_SYS_BASE + PLL_PWR) &= ~(1u << PLL_PWR_POSTDIVPD_BIT);
}

static void set_clk_sys_pll(void) {
    uint32_t clk_sys_ctrl = REG(CLOCKS_BASE + CLOCKS_CLK_SYS_CTRL);
    clk_sys_ctrl &= ~CLOCKS_CLK_SYS_CTRL_AUXSRC_MASK;
    clk_sys_ctrl |= CLOCKS_CLK_SYS_CTRL_CLKSRC_PLL_SYS << CLOCKS_CLK_SYS_CTRL_AUXSRC_LSB;
    REG(CLOCKS_BASE + CLOCKS_CLK_SYS_CTRL) = clk_sys_ctrl;

    clk_sys_ctrl &= ~CLOCKS_CLK_SYS_CTRL_SRC_MASK;
    clk_sys_ctrl |= CLOCKS_CLK_SYS_CTRL_CLKSRC_CLK_SYS_AUX;
    REG(CLOCKS_BASE + CLOCKS_CLK_SYS_CTRL) = clk_sys_ctrl;

    while (!(REG(CLOCKS_BASE + CLOCKS_CLK_SYS_SELECTED)
             & (1u << CLOCKS_CLK_SYS_CTRL_CLKSRC_CLK_SYS_AUX))) {;}
}

static uint32_t fc0_measure_khz(uint32_t src) {
    uint32_t fc0_src = REG(CLOCKS_BASE + CLOCKS_FC0_SRC);
    fc0_src &= ~CLOCKS_FC0_SRC_MASK;
    fc0_src |= src;
    REG(CLOCKS_BASE + CLOCKS_FC0_SRC) = fc0_src;

    while (!(REG(CLOCKS_BASE + CLOCKS_FC0_STATUS) & (1u << CLOCKS_FC0_STATUS_DONE))) {;}

    return REG(CLOCKS_BASE + CLOCKS_FC0_RESULT) >> CLOCKS_FC0_RESULT_KHZ_LSB;
}

static void configure_clk_peri(void){
    // disable clk_peri
    REG(CLOCKS_BASE + CLOCKS_CLK_PERI_CTRL) &= ~(1u << CLOCKS_CLK_PERI_CTRL_ENABLE_BIT);

    // wire to xosc
    uint32_t clk_peri_ctrl = REG(CLOCKS_BASE + CLOCKS_CLK_PERI_CTRL); 
    clk_peri_ctrl &= ~CLOCKS_CLK_PERI_CTRL_AUXSRC_MASK;
    clk_peri_ctrl |= CLOCKS_CLK_PERI_CTRL_XOSC_CLKSRC << CLOCKS_CLK_PERI_CTRL_AUXSRC_LSB; 
    REG(CLOCKS_BASE + CLOCKS_CLK_PERI_CTRL) = clk_peri_ctrl;

    // enable clk_peri
    REG(CLOCKS_BASE + CLOCKS_CLK_PERI_CTRL) |= (1u << CLOCKS_CLK_PERI_CTRL_ENABLE_BIT);
}

int main(){
    REG(PPB_BASE + SCB_VTOR) = (uint32_t)&__vectors_start;
    REG(PPB_BASE + NVIC_ISER) = (1u << NVIC_ISER_TIMER_IRQ_0);

    REG(RESETS_BASE + APB_ATOMIC_CLR + RESETS_RESET) = (1u << RESETS_RESET_IO_BANK0_LSB)
                                                     | (1u << RESETS_RESET_PADS_BANK0_LSB)
                                                     | (1u << RESETS_RESET_TIMER)
                                                     | (1u << RESETS_RESET_PLL_SYS_LSB);

    while( !(REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_IO_BANK0_LSB)) ){;}
    while( !(REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_PADS_BANK0_LSB)) ){;}
    while( !( REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_TIMER) )) {;}
    while( !( REG(RESETS_BASE + RESETS_RESET_DONE) & (1u << RESETS_RESET_PLL_SYS_LSB) )) {;}

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

    pll_sys_init();
    set_clk_sys_pll();


    configure_clk_peri();

    // alarm_set();

    uint32_t is_freq_valid = is_freq_in_range(DEFUALT_CLOCK_KHZ, fc0_measure_khz(CLOCKS_FC0_SRC_CLK_PERI), 1);
    if(is_freq_valid){
         for(;;){
             blink();
             blink();
             spin(GAP_SPIN);
         }
    } else{
         for(;;){
             blink();
             spin(GAP_SPIN);
         }
    }
}
