CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc


SIZE = arm-none-eabi-size


C_SOURCES = \
	core/main.c \
	core/freertos_heap.c \
	core/system_panic.c \
	core/hooks.c \
	core/memory.c \
	profiler/cycle_counter.c \
	profiler/timer.c \
	profiler/task_profiler.c \
	freertos/FreeRTOS-Kernel/tasks.c \
	freertos/FreeRTOS-Kernel/queue.c \
	freertos/FreeRTOS-Kernel/list.c \
	freertos/FreeRTOS-Kernel/timers.c \
	freertos/FreeRTOS-Kernel/event_groups.c \
	freertos/FreeRTOS-Kernel/portable/MemMang/heap_4.c \
	freertos/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c

ASM_SOURCES = startup/startup_stm32f411.s

OBJECTS = \
	build/main.o \
	build/freertos_heap.o \
	build/system_panic.o \
	build/hooks.o \
	build/cycle_counter.o \
	build/timer.o \
	build/tasks.o \
	build/queue.o \
	build/list.o \
	build/timers.o \
	build/event_groups.o \
	build/heap_4.o \
	build/port.o \
	build/startup_stm32f411.o \
	build/memory.o \
	build/task_profiler.o 

OBJCOPY = arm-none-eabi-objcopy

ARCH_FLAGS = \
	-mcpu=cortex-m4 \
	-mthumb \
	-mfpu=fpv4-sp-d16 \
	-mfloat-abi=hard

INCLUDES = \
	-Iconfig \
	-Icore \
	-Iprofiler \
	-Ifreertos/FreeRTOS-Kernel/include \
	-Ifreertos/FreeRTOS-Kernel/portable/GCC/ARM_CM4F



CFLAGS = \
	$(ARCH_FLAGS) \
	-std=c11 \
	-g \
	-O0 \
	-Wall \
	-Wextra \
	-ffreestanding \
	-ffunction-sections \
	-fdata-sections \
	$(INCLUDES)

LDFLAGS = $(ARCH_FLAGS) \
          -nostdlib \
		  -T linker/stm32f411.ld \
		  -Wl,-Map=build/firmware.map \
		  -Wl,--gc-sections \
		  -Wl,--print-memory-usage


all:build/firmware.elf \
	build/firmware.bin \
	build/firmware.hex \
	


build:
	mkdir -p build

build/main.o: core/main.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/freertos_heap.o: core/freertos_heap.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/system_panic.o: core/system_panic.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/hooks.o: core/hooks.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/cycle_counter.o: profiler/cycle_counter.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/timer.o: profiler/timer.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/task_profiler.o: profiler/task_profiler.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/memory.o: core/memory.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/tasks.o: freertos/FreeRTOS-Kernel/tasks.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/queue.o: freertos/FreeRTOS-Kernel/queue.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/list.o: freertos/FreeRTOS-Kernel/list.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/timers.o: freertos/FreeRTOS-Kernel/timers.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/event_groups.o: freertos/FreeRTOS-Kernel/event_groups.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/heap_4.o: freertos/FreeRTOS-Kernel/portable/MemMang/heap_4.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/port.o: freertos/FreeRTOS-Kernel/portable/GCC/ARM_CM4F/port.c | build
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