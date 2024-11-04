#ifndef PIT_DRIVER_H
#define PIT_DRIVER_H
#include <stdint.h>

// Initializes the PIT, setting the frequency to 
void init_pit();

void sleep(uint64_t milis);

uint64_t ticks_elapsed();
uint64_t milliseconds_elapsed();

#endif