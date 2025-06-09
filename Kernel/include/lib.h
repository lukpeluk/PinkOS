#ifndef LIB_H
#define LIB_H

#include <stdint.h>
#include <stdint.h>

void * memset(void * destination, int32_t character, uint64_t length);
void * memcpy(void * destination, const void * source, uint64_t length);

void itoa(int value, char *str, int base);

char *cpuVendor(char *result);

#endif