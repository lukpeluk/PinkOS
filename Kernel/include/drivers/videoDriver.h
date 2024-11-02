#ifndef	_VIDEO_DRIVER_H__
#define _VIDEO_DRIVER_H__
#include <stdint.h>

typedef struct {
    uint64_t x;
    uint64_t y;
} Point;            // Flashbacks de POO

typedef uint8_t **Font;

// BASIC SHAPES
void putPixel(uint32_t hexColor, uint64_t x, uint64_t y);

void drawRectangle(Point * start, Point * end, uint32_t hexColor);
void drawRectangleBoder(Point * start, Point * end, uint32_t thickness, uint32_t hexColor);

// TEXT AND NUMBERS

// sabe la posición del último caracter dibujado y dibuja el siguiente
// hace wrapping automático, aunque debería ser configurable con un flag
void drawChar(unsigned char c, uint32_t textColor, uint32_t bgColor, int wrap);
void drawString(char * string, uint32_t textColor, uint32_t bgColor);
void drawCharAt(char c, uint32_t textColor, uint32_t bgColor, Point * position);
void drawStringAt(char * string, uint32_t textColor, uint32_t bgColor, Point * position);

void drawNumber(uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap);
void drawNumberAt(uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position);
void drawHex(uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap);
void drawHexAt(uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position);

// BITMAPS

// draw a bitmap image at a given position
void drawBitmap(uint32_t * bitmap, uint64_t width, uint64_t height, Point * position, uint32_t scale); 

// CURSOR

void setCursorLine(uint32_t line);
void setCursorColumn(uint32_t column);

uint32_t getCursorLine();
uint32_t getCursorColumn();

int isCursorInBoundaries(uint32_t line, uint32_t column);

uint64_t getScreenWidth();
uint64_t getScreenHeight();
uint64_t getCharWidth();
uint64_t getCharHeight();

// GENERAL

void clearScreen(uint32_t bgColor);

// range: 1-8
void setFontSize(uint8_t fontSize);
void setFont(Font font); 

#endif
