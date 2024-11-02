#ifndef GRAPHICSLIB_H
#define GRAPHICSLIB_H

#include <stdint.h>

typedef struct {
	uint64_t x;
	uint64_t y;
} Point;

uint64_t getScreenWidth();
uint64_t getScreenHeight();
uint64_t getCharWidth();
uint64_t getCharHeight();

void drawPixel(uint32_t color, Point position);
void drawRectangle(uint32_t color, int width, int height, Point position);
void drawRectangleBoder(uint32_t color, int width, int height, int border_width, Point position);

void drawChar(char c, uint32_t color, uint32_t bgColor, Point position);
void drawString(char * string, uint32_t color, uint32_t bgColor, Point position);
void drawNumber(uint64_t num, uint32_t color, uint32_t bgColor, Point position);
void drawHex(uint64_t num, uint32_t color, uint32_t bgColor, Point position);

void drawBitmap(uint32_t * bitmap, uint64_t width, uint64_t height, Point position, uint32_t scale);

void clearScreen(uint32_t color);

#endif