#include <stdint.h>
#include "FreeRTOS.h"

uint8_t ucHeap[configTOTAL_HEAP_SIZE]
    __attribute__((section(".heap"),aligned(8)));
