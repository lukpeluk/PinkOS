#ifndef NAIVE_CONSOLE_H
#define NAIVE_CONSOLE_H

#include <stdint.h>

void ncPrint(const unsigned char * string);
void ncPrintColor(const unsigned char * string, unsigned char color);
void ncPrintChar(unsigned char character);
void ncPrintCharColor(unsigned char character, unsigned char color);
void ncNewline();
void ncPrintDec(uint64_t value);
void ncPrintHex(uint64_t value);
void ncPrintBin(uint64_t value);
void ncPrintBase(uint64_t value, uint32_t base);
void ncClear();

#endif