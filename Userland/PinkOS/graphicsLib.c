#include <graphicsLib.h>
#include <syscallCodes.h>
#include <stdpink.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

uint64_t getScreenWidth(){
    uint64_t screen_width;
    syscall(GET_SCREEN_WIDTH_SYSCALL, &screen_width, 0, 0, 0, 0);
    return screen_width;
}

uint64_t getScreenHeight(){
    uint64_t screen_height;
    syscall(GET_SCREEN_HEIGHT_SYSCALL, &screen_height, 0, 0, 0, 0);
    return screen_height;
}

uint64_t getCharWidth(){
    uint64_t char_width;
    syscall(GET_CHAR_WIDTH_SYSCALL, &char_width, 0, 0, 0, 0);
    return char_width;
}

uint64_t getCharHeight(){
    int char_height;
    syscall(GET_CHAR_HEIGHT_SYSCALL, &char_height, 0, 0, 0, 0);
    return char_height;
}


void clearScreen(uint32_t color){
    syscall(CLEAR_SCREEN_SYSCALL, color, 0, 0, 0, 0);
}

void drawPixel(uint32_t color, Point position){
    syscall(DRAW_PIXEL_SYSCALL, color, position.x, position.y, 0, 0);
}

void drawRectangle(uint32_t color, int width, int height, Point position){
    Point start = position;
    Point end = {position.x + width, position.y + height};
    syscall(DRAW_RECTANGLE_SYSCALL, &start, &end, color, 0, 0);
}

void drawRectangleBorder(uint32_t color, int width, int height, int border_width, Point position){
    Point start = position;
    Point end = {position.x + width, position.y + height};
    syscall(DRAW_RECTANGLE_BORDER_SYSCALL, &start, &end, border_width, color, 0);
}

void drawChar(unsigned char c, uint32_t color, uint32_t bgColor, Point position){
    syscall(DRAW_CHAR_AT_SYSCALL, c, color, bgColor, &position, 0);
}

void drawString(unsigned char * string, uint32_t color, uint32_t bgColor, Point position){
    syscall(DRAW_STRING_AT_SYSCALL, string, color, bgColor, &position, 0);
}

void drawNumber(uint64_t num, uint32_t color, uint32_t bgColor, Point position){
    syscall(DRAW_NUMBER_AT_SYSCALL, num, color, bgColor, &position, 0);
}

void drawHex(uint64_t num, uint32_t color, uint32_t bgColor, Point position){
    syscall(DRAW_HEX_AT_SYSCALL, num, color, bgColor, &position, 0);
}

void drawBitmap(uint32_t * bitmap, uint64_t width, uint64_t height, Point position, uint32_t scale){
    syscall(DRAW_BITMAP_SYSCALL, bitmap, width, height, &position, scale);
}

// void drawBitmapTransparent(uint32_t * bitmap, uint64_t width, uint64_t height, Point position, uint32_t scale){
//     syscall(DRAW_BITMAP_TRANSPARENT_SYSCALL, bitmap, width, height, &position, scale);
// }