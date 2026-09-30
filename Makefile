# Bare-metal RP2040 — no SDK, no CMake, every flag visible.

CC      = arm-none-eabi-gcc
OBJDUMP = arm-none-eabi-objdump
OBJCOPY = arm-none-eabi-objcopy
READELF = arm-none-eabi-readelf
NM      = arm-none-eabi-nm
SIZE    = arm-none-eabi-size
HOSTCC  = cc
LINK   ?= sram.ld

# -mcpu/-mthumb : Cortex-M0+ has no ARM mode, only Thumb
# -nostdlib     : no libc, no crt0 from the toolchain — we supply our own
# -ffreestanding: there is no OS; don't assume main() has the hosted meaning
# -O1 -g        : optimised enough to be realistic, with debug info for CP8
CFLAGS  = -mcpu=cortex-m0plus -mthumb -nostdlib -ffreestanding -Wall -Wextra -O1 -g

SRCS   = fw/crt0.s fw/main.c fw/vectors.c
LDFILE = fw/$(LINK)

TARGET_NAME = build/main-$(basename $(LINK))
ELF         = $(TARGET_NAME).elf
BIN         = $(TARGET_NAME).bin
PATCHED_BIN = $(TARGET_NAME)-crc.bin
MAP         = $(TARGET_NAME).map
CRC         = build/crc32

# -T           : our linker script decides every address
# -Wl,-Map     : write a map file — the record of what landed where, and why
LDFLAGS = -T $(LDFILE) -Wl,-Map=$(MAP)

# boot2 and the CRC32 at 0xfc exist only for the flash layout. The SRAM image
# reserves nothing at that offset, so patching it would overwrite real code.
ifeq ($(LINK),flash.ld)
  SRCS  += fw/boot2.s
  IMAGE  = $(PATCHED_BIN)
else
  IMAGE  =
endif

.PHONY: all inspect disasm pico-ping load clean flash flash-load crc

all: $(ELF) $(IMAGE)

$(ELF): $(SRCS) $(LDFILE)
	@mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) -o $@
	$(SIZE) $@

$(BIN): $(ELF)
	$(OBJCOPY) -O binary $< $@

$(PATCHED_BIN): $(BIN) $(CRC)
	$(CRC) $< $@

$(CRC): tools/crc32.c
	@mkdir -p build
	$(HOSTCC) -Wall -Wextra -O2 -o $@ $<

crc: $(CRC)

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

# A raw .bin carries no addresses — objcopy stripped them — so the flash path
# has to say where the image goes. The ELF still carries its own.
ifeq ($(LINK),flash.ld)
load: $(PATCHED_BIN)
	picotool load -x $< -t bin -o 0x10000000
else
load: $(ELF)
	picotool load -x $<
endif

flash:
	$(MAKE) LINK=flash.ld

flash-load:
	$(MAKE) LINK=flash.ld load

clean:
	rm -rf build
