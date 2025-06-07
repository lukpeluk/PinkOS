#ifndef	_VIDEO_DRIVER_H__
#define _VIDEO_DRIVER_H__
#include <stdint.h>

#define IS_PRINTABLE_CHAR(ascii) (ascii >= 32 && ascii < 255 && ascii != 127)

typedef struct {
    uint64_t x;
    uint64_t y;
} Point;            // Flashbacks de POO

typedef uint8_t **Font;

// Video driver initialization
void initVideoDriver();

// main loop (intended to be called every timer tick, to manage the video stream)
// This function is responsible for updating the video buffer and rendering it to the screen.
void videoLoop();

// Buffer initialization
// Returns a pointer to the video buffer, which is a 2D array of pixels. Each pixel is represented by a 32-bit integer.
uint8_t * createVideoBuffer();

// BASIC SHAPES
void putPixel(uint8_t videoBuffer, uint32_t hexColor, uint64_t x, uint64_t y);

void drawRectangle(uint8_t videoBuffer, Point * start, Point * end, uint32_t hexColor);
void drawRectangleBoder(uint8_t videoBuffer, Point * start, Point * end, uint32_t thickness, uint32_t hexColor);

// TEXT AND NUMBERS

// sabe la posición del último caracter dibujado y dibuja el siguiente
// hace wrapping automático, aunque debería ser configurable con un flag
void drawChar(uint8_t videoBuffer, char c, uint32_t textColor, uint32_t bgColor, int wrap);
void drawString(uint8_t videoBuffer, char * string, uint32_t textColor, uint32_t bgColor);
void drawCharAt(uint8_t videoBuffer, char c, uint32_t textColor, uint32_t bgColor, Point * position);
void drawStringAt(uint8_t videoBuffer, char * string, uint32_t textColor, uint32_t bgColor, Point * position);

void drawNumber(uint8_t videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap);
void drawNumberAt(uint8_t videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position);
void drawHex(uint8_t videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap);
void drawHexAt(uint8_t videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position);

// BITMAPS

// draw a bitmap image at a given position
void drawBitmap(uint8_t videoBuffer, uint32_t * bitmap, uint64_t width, uint64_t height, Point * position, uint32_t scale); 

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

void clearScreen(uint8_t videoBuffer, uint32_t bgColor);

// range: 1-8
void setFontSize(uint8_t fontSize);
void setFont(Font font); 
void incFontSize();
void decFontSize();

#endif
