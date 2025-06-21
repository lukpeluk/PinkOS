#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>
#include <stddef.h>

// #define USE_BUDDY

void initMemoryManager();
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t new_size);

#endif