#ifndef	_VIDEO_DRIVER_H__
#define _VIDEO_DRIVER_H__
#include <stdint.h>

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y);

// sabe la posición del último caracter dibujado y dibuja el siguiente
// hace wrapping automático, aunque debería ser configurable con un flag
void drawChar(char c, uint32_t textColor, uint32_t bgColor);

void setCursorLine(uint32_t line);

void setCursorColumn(uint32_t column);

void clearScreen(uint32_t bgColor);

#endif
