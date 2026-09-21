# rp2040-no-sdk-cold-start

Bare-metal RP2040 bring-up. No pico-sdk, no HAL, no CMake — every instruction from reset
to `main()` is mine, written from the datasheet against raw registers.

**The one rule:** never call a function I didn't write.

**Why:** "what happens before `main()`" was a black box. This is me opening it.

## Project goals status

- [x] A binary that runs — linker script, `crt0.s`, ELF that boots into SRAM
- [x] Blink an LED with four register writes — resets, pad mux, GPIO direction, SIO
- [x] `registers.h` — named addresses and bitfields, no magic numbers
- [x] `.data` and `.bss` — copy and zero them myself, because C promises what silicon doesn't
- [x] Vector table — hand-built, loaded into `VTOR`
- [x] First interrupt — hardware timer alarm drives the blink
- [x] HardFault handler — faults announce themselves instead of hanging
- [x] Frequency counter — measure the clock tree instead of trusting it
- [x] Crystal oscillator up, `clk_ref` switched onto it
- [x] Watchdog tick — the timer's microseconds are now actually microseconds
- [ ] PLL to 125 MHz, `clk_sys` switched onto it
- [ ] `boot2` — cold boot from flash, standing on its own
- [ ] SWD debug probe
- [ ] I²C from the datasheet
- [ ] Display driver

## Build

```
make            # -> build/main.elf
make load       # picotool load -x
make pico-ping  # is the ROM bootloader in charge, or is my code running?
make disasm     # what the compiler actually emitted
```

Host tools (`picotool`, `gdb`) run on the Mac, not on the chip.
