#include <stdint.h>
#include <types.h>
#include <memoryManager/memoryManager.h>
#include <drivers/serialDriver.h>

void * memset(void * destination, int32_t c, uint64_t length)
{
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destination;

	while(length--)
		dst[length] = chr;

	return destination;
}

//* Nota: tenemos una implementación más rapida en assembler en libasm.asm, lightspeed_memcpy
void * memcpy(void * destination, const void * source, uint64_t length)
{
	/*
	* memcpy does not support overlapping buffers, so always do it
	* forwards. (Don't change this without adjusting memmove.)
	*
	* For speedy copying, optimize the common case where both pointers
	* and the length are word-aligned, and copy word-at-a-time instead
	* of byte-at-a-time. Otherwise, copy by bytes.
	*
	* The alignment logic below should be portable. We rely on
	* the compiler to be reasonably intelligent about optimizing
	* the divides and modulos out. Fortunately, it is.
	*/
	uint64_t i;

	if ((uint64_t)destination % sizeof(uint32_t) == 0 &&
		(uint64_t)source % sizeof(uint32_t) == 0 &&
		length % sizeof(uint32_t) == 0)
	{
		uint32_t *d = (uint32_t *) destination;
		const uint32_t *s = (const uint32_t *)source;

		for (i = 0; i < length / sizeof(uint32_t); i++)
			d[i] = s[i];
	}
	else
	{
		uint8_t * d = (uint8_t*)destination;
		const uint8_t * s = (const uint8_t*)source;

		for (i = 0; i < length; i++)
			d[i] = s[i];
	}

	return destination;
}

// Int to ascii
void itoa(int value, char *str, int base) {
    char *p = str;
    int num = value;
    int sign = 0;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        sign = 1;
        num = -value;
    }

    char buf[32];
    int i = 0;

    while (num != 0) {
        int digit = num % base;
        buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num /= base;
    }

    if (sign)
        *p++ = '-';

    while (i--)
        *p++ = buf[i];

    *p = '\0';
}



// String functions (OJO: tienen que estar bien los strings, si no están null terminated o no tenés espacio suficiente podés hacer cagada fuerte)

// Va a estar antes en el orden un string que sea más corto
int strcmp(const char *str1, const char *str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

// Supone que hay espacio suficiente en dest
void strcpy(char *dest, const char *src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

int strlen(const char *str) {
    int len = 0;
    while (*str) {
        len++;
        str++;
    }
    return len;
}

char *strdup(const char *str) {
    char * new = malloc(strlen((char *)str) + 1);
    if (new == NULL)
        return NULL;
    
    strcpy(new, (char *)str);
    return new;
}

int are_interrupts_enabled() {
    unsigned long flags;
    __asm__ volatile("pushf; pop %0" : "=r"(flags));
    return (flags & 0x200) != 0;
}

void panic_if_ints_enabled() {
    if(are_interrupts_enabled()) {
        log_to_serial("E: PANIC CALLED WITH INTERRUPTS ENABLED! ");
    }
}