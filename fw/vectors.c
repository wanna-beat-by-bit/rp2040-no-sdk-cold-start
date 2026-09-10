#include <stdint.h>

extern uint32_t __stack_top;

typedef union {
    void (*handler)(void); 
    void *stack;
} vector_t;

void _start(void);
void Default_Handler(void) { while(1); }

// standard exceptions
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));


void TIMER_IRQ_0_Handler(void) __attribute__((weak, alias("Default_Handler")));   // 0: Timer 0
void TIMER_IRQ_1_Handler(void) __attribute__((weak, alias("Default_Handler")));   // 1: Timer 1
void TIMER_IRQ_2_Handler(void) __attribute__((weak, alias("Default_Handler")));   // 2: Timer 2
void TIMER_IRQ_3_Handler(void) __attribute__((weak, alias("Default_Handler")));   // 3: Timer 3
void PWM_IRQ_WRAP_Handler(void) __attribute__((weak, alias("Default_Handler")));  // 4: PWM wrap counter
void USBCTRL_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));   // 5: USB Controller
void XIP_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));       // 6: XIP cache
void PIO0_IRQ_0_Handler(void) __attribute__((weak, alias("Default_Handler")));    // 7: PIO 0 Interrupt 0
void PIO0_IRQ_1_Handler(void) __attribute__((weak, alias("Default_Handler")));    // 8: PIO 0 Interrupt 1
void PIO1_IRQ_0_Handler(void) __attribute__((weak, alias("Default_Handler")));    // 9: PIO 1 Interrupt 0
void PIO1_IRQ_1_Handler(void) __attribute__((weak, alias("Default_Handler")));    // 10: PIO 1 Interrupt 1
void DMA_IRQ_0_Handler(void) __attribute__((weak, alias("Default_Handler")));     // 11: DMA Interrupt 0
void DMA_IRQ_1_Handler(void) __attribute__((weak, alias("Default_Handler")));     // 12: DMA Interrupt 1
void IO_IRQ_BANK0_Handler(void) __attribute__((weak, alias("Default_Handler")));  // 13: GPIO Bank 0
void IO_IRQ_QSPI_Handler(void) __attribute__((weak, alias("Default_Handler")));   // 14: QSPI Flash GPIO
void SIO_IRQ_PROC0_Handler(void) __attribute__((weak, alias("Default_Handler"))); // 15: Core 0 Inter-process FIFO
void SIO_IRQ_PROC1_Handler(void) __attribute__((weak, alias("Default_Handler"))); // 16: Core 1 Inter-process FIFO
void CLOCKS_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));    // 17: Clock Generator
void SPI0_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));      // 18: SPI 0
void SPI1_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));      // 19: SPI 1
void UART0_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));     // 20: UART 0
void UART1_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));     // 21: UART 1
void ADC_IRQ_FIFO_Handler(void) __attribute__((weak, alias("Default_Handler")));  // 22: ADC FIFO pre-fetch
void I2C0_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));      // 23: I2C 0
void I2C1_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));      // 24: I2C 1
void RTC_IRQ_Handler(void) __attribute__((weak, alias("Default_Handler")));       // 25: Real Time Clock

__attribute__((section(".vectors"), used))
const vector_t vectors[] = {
    {.stack = &__stack_top},
    {.handler = _start},
    {.handler = NMI_Handler},
    {.handler = HardFault_Handler},
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {.handler = SVC_Handler},
    {0},
    {0},
    {.handler = PendSV_Handler},
    {.handler = SysTick_Handler},

    {.handler=TIMER_IRQ_0_Handler},
    {.handler=TIMER_IRQ_1_Handler},
    {.handler=TIMER_IRQ_2_Handler},
    {.handler=TIMER_IRQ_3_Handler},
    {.handler=PWM_IRQ_WRAP_Handler},
    {.handler=USBCTRL_IRQ_Handler},
    {.handler=XIP_IRQ_Handler},
    {.handler=PIO0_IRQ_0_Handler},
    {.handler=PIO0_IRQ_1_Handler},
    {.handler=PIO1_IRQ_0_Handler},
    {.handler=PIO1_IRQ_1_Handler},
    {.handler=DMA_IRQ_0_Handler},
    {.handler=DMA_IRQ_1_Handler},
    {.handler=IO_IRQ_BANK0_Handler},
    {.handler=IO_IRQ_QSPI_Handler},
    {.handler=SIO_IRQ_PROC0_Handler},
    {.handler=SIO_IRQ_PROC1_Handler},
    {.handler=CLOCKS_IRQ_Handler},
    {.handler=SPI0_IRQ_Handler},
    {.handler=SPI1_IRQ_Handler},
    {.handler=UART0_IRQ_Handler},
    {.handler=UART1_IRQ_Handler},
    {.handler=ADC_IRQ_FIFO_Handler},
    {.handler=I2C0_IRQ_Handler},
    {.handler=I2C1_IRQ_Handler},
    {.handler=RTC_IRQ_Handler},
};
