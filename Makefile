CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc


SIZE = arm-none-eabi-size

C_SOURCES = core/main.c \
			profiler/cycle_counter.c \
			profiler/profiler.c

ASM_SOURCES = startup/startup_stm32f411.s

OBJECTS = \
			build/main.o \
			build/cycle_counter.o \
			build/startup_stm32f411.o \
			build/profiler.o

OBJCOPY = arm-none-eabi-objcopy
ARCH_FLAGS = -mcpu=cortex-m4 -mthumb

CFLAGS = $(ARCH_FLAGS) -g -O0 -Wall
CFLAGS += -ffreestanding
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -Iprofiler

LDFLAGS = $(ARCH_FLAGS) \
          -nostdlib \
		  -T linker/stm32f411.ld \
		  -Wl,-Map=build/firmware.map \
		  -Wl,--gc-sections

all:build/firmware.elf \
	build/firmware.bin \
	build/firmware.hex \
	


build:
	mkdir -p build

build/main.o: core/main.c | build 
	$(CC) $(CFLAGS) -c $< -o $@

build/cycle_counter.o: profiler/cycle_counter.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/profiler.o: profiler/profiler.c | build
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