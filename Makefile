CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc

CFLAGS = -mcpu=cortex-m4 -mthumb -g -O0 -Wall
SIZE = arm-none-eabi-size

CFLAGS += -ffreestanding
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections

LDFLAGS = -T linker/stm32f411.ld \
		-Wl,-Map=build/firmware.map \
		-Wl,--gc-sections

C_SOURCES = core/main.c
ASM_SOURCES = startup/startup_stm32f411.s

OBJECTS = \
			build/main.o \
			build/startup_stm32f411.o

OBJCOPY = arm-none-eabi-objcopy

all:build/firmware.elf \
	build/firmware.bin \
	build/firmware.hex \
	


build:
	mkdir -p build

build/main.o: core/main.c | build 
	$(CC) $(CFLAGS) -c $< -o $@

build/startup_stm32f411.o: startup/startup_stm32f411.s | build 
	$(CC) $(CFLAGS) -c $< -o $@

build/firmware.elf: $(OBJECTS) | build
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

build/firmware.bin:  build/firmware.elf
	$(OBJCOPY) -O binary $< $@

build/firmware.hex:  build/firmware.elf
	$(OBJCOPY) -O ihex $< $@


clean: 
	rm -rf build

.PHONY: clean all