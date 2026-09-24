# Bare-metal RP2040 — no SDK, no CMake, every flag visible.

CC      = arm-none-eabi-gcc
OBJDUMP = arm-none-eabi-objdump
READELF = arm-none-eabi-readelf
NM      = arm-none-eabi-nm
SIZE    = arm-none-eabi-size
LINK ?= sram.ld

# -mcpu/-mthumb : Cortex-M0+ has no ARM mode, only Thumb
# -nostdlib     : no libc, no crt0 from the toolchain — we supply our own
# -ffreestanding: there is no OS; don't assume main() has the hosted meaning
# -O1 -g        : optimised enough to be realistic, with debug info for CP8
CFLAGS  = -mcpu=cortex-m0plus -mthumb -nostdlib -ffreestanding -Wall -Wextra -O1 -g

# -T           : our linker script decides every address
# -Wl,-Map     : write a map file — the record of what landed where, and why
LDFLAGS = -T fw/$(LINK) -Wl,-Map=build/main.map

SRCS   = fw/crt0.s fw/main.c fw/vectors.c
LDFILE = fw/$(LINK)
ELF = build/main-$(basename $(LINK)).elf

.PHONY: all inspect disasm pico-ping load clean

all: $(ELF)

$(ELF): $(SRCS) $(LDFILE)
	@mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $@
	$(SIZE) $@

# Did the linker put things where the script said? The check you can't do by reading code.
inspect: $(ELF)
	@echo "=== entry point (odd address = Thumb bit, correct) ==="
	@$(READELF) -h $(ELF) | grep -i 'entry point'
	@echo
	@echo "=== segments: VirtAddr = where it runs, PhysAddr = where it loads ==="
	@$(READELF) -l $(ELF) | sed -n '/Program Headers/,/^$$/p'
	@echo "=== symbols, by address ==="
	@$(NM) -n $(ELF)

disasm: $(ELF)
	$(OBJDUMP) -d $(ELF)

# Is the ROM's USB bootloader still in charge? Reads the live USB tree, not
# /Volumes, because Finder caches the mount after the device is gone.
# Present  -> ROM running, ready to load.
# Absent   -> your code took over. That is the CP1 success signal.
pico-ping:
	@if ioreg -p IOUSB -w0 -l | grep -q '"USB Product Name" = "RP2 Boot"'; then \
		echo "PRESENT  — RP2 Boot on USB, ROM bootloader in charge"; \
	else \
		echo "ABSENT   — no RP2 bootloader; your code is running (or board unplugged)"; \
	fi

load: $(ELF)
	picotool load -x $(ELF)

clean:
	rm -rf build
