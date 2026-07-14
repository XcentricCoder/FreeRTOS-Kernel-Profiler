#ifndef CYCLECOUNTER_H
#define CYCLECOUNTER_H

#include <stdint.h>
#include <stdbool.h>

bool cycle_counter_init(void);
uint32_t cycle_counter_get(void);


#endif