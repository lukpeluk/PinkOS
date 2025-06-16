#ifndef LIB_H
#define LIB_H

#include <stdint.h>

void * memset(void * destination, int32_t character, uint64_t length);
void * memcpy(void * destination, const void * source, uint64_t length);

/**
 * @brief Copies memory from source to destination, optimized for speed.
 * @param dest Pointer to the destination memory.
 * @param src Pointer to the source memory.
 * @param len Number of bytes to copy.
 * @return Pointer to the destination memory.
 */
void *lightspeed_memcpy(void *dest, const void *src, uint64_t len);

void itoa(int value, char *str, int base);

int strcmp(const char *str1, const char *str2);
void strcpy(char *dest, const char *src);
int strlen(const char *str);
char* strdup(const char *str);

char *cpuVendor(char *result);

#endif