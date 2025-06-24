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

/**
 * @brief Sets memory to a specific value, optimized for speed.
 * @param dest Pointer to the destination memory.
 * @param value Value to set (only the least significant byte is used).
 * @param len Number of bytes to set.
 * @return Pointer to the destination memory.
 */
void *lightspeed_memset(void *dest, int value, uint64_t len);

/**
 * @brief Sets memory to zero, optimized for speed.
 * @param dest Pointer to the destination memory.
 * @param len Number of bytes to zero.
 * @return Pointer to the destination memory.
 */
void *lightspeed_memzero(void *dest, uint64_t len);

/**
 * @brief Compares two memory regions, optimized for speed.
 * @param s1 Pointer to the first memory region.
 * @param s2 Pointer to the second memory region.
 * @param len Number of bytes to compare.
 * @return 0 if equal, <0 if s1<s2, >0 if s1>s2.
 */
int lightspeed_memcmp(const void *s1, const void *s2, uint64_t len);

/**
 * @brief Moves memory from source to destination, handling overlaps correctly.
 * @param dest Pointer to the destination memory.
 * @param src Pointer to the source memory.
 * @param len Number of bytes to move.
 * @return Pointer to the destination memory.
 */
void *lightspeed_memmove(void *dest, const void *src, uint64_t len);

void itoa(int value, char *str, int base);

int strcmp(const char *str1, const char *str2);
void strcpy(char *dest, const char *src);
int strlen(const char *str);
char* strdup(const char *str);

int are_interrupts_enabled();

char *cpuVendor(char *result);

#endif