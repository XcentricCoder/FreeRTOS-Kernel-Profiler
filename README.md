# FreeRTOS Kernel Profiler

A bare-metal FreeRTOS kernel profiler for the STM32F411RE.

## Goals

- Port FreeRTOS without CubeMX
- Build startup code from scratch
- Write a custom linker script
- Instrument PendSV and SysTick
- Profile task execution time using DWT CYCCNT

## Hardware

- STM32F411RE Nucleo

## Toolchain

- arm-none-eabi-gcc
- GNU Make
- OpenOCD
- GDB

## Current Progress

- [x] Linker Script
- [x] Startup Assembly
- [x] Build System
- [ ] CMSIS Integration
- [ ] UART Driver
- [ ] FreeRTOS Port
- [ ] Kernel Profiler