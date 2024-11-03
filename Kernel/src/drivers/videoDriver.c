#include <drivers/videoDriver.h>
#include <drivers/defaultFont.h>
#include <stdint.h>

struct vbe_mode_info_structure {
	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;			// number of bytes per horizontal line
	uint16_t width;			// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;			// unused...
	uint8_t y_char;			// ...
	uint8_t planes;
	uint8_t bpp;			// bits per pixel in this mode
	uint8_t banks;			// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;

	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;

	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];
} __attribute__ ((packed));

typedef struct vbe_mode_info_structure * VBEInfoPtr;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;
static Font font = ibm_bios_font;
static font_size = 2;

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
	if(x >= VBE_mode_info->width || y >= VBE_mode_info->height){
		return;
	}
	uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
	uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
	framebuffer[offset]     =  (hexColor) & 0xFF;
	framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
	framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

// BASIC SHAPES

void drawRectangle(Point * start, Point * end, uint32_t hexColor){
	for(uint64_t i = start->x; i < end->x; i++){
		for(uint64_t j = start->y; j < end->y; j++){
			putPixel(hexColor, i, j);
		}
	}
}

void drawRectangleBoder(Point * start, Point * end, uint32_t thickness, uint32_t hexColor){
	for(uint64_t i = start->x; i < end->x; i++){
		for(uint64_t j = start->y; j < end->y; j++){
			if(i < start->x + thickness || i > end->x - thickness || j < start->y + thickness || j > end->y - thickness){
				putPixel(hexColor, i, j);
			}
		}
	}
}


#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define FONT_NOF_CHARS 95

static uint64_t x = 0;
static uint64_t y = 0;
static char INTERLINE = 3;

// funca con caracteres imprimibles soportados por la tipografía, y con el salto de línea y delete
// hace wrapping automático, podría configurarse con un flag
// para borrar le escribe un espacio en blanco por arriba
void drawChar(unsigned char c, uint32_t textColor, uint32_t bgColor, int wrap) {
    int is_deleting = 0;

	// salto de línea (antes del wrapping pues este no le debe afectar)
	if(c == '\n'){
        x = 0;
		y += (CHAR_HEIGHT * font_size) + INTERLINE;
		return;
	}

    // no tiene sentido un wrapping vertical, así que si se pasa del borde de la pantalla para abajo, no imprime nada
    if(y > VBE_mode_info->height){
        return;
    }
    
	// hago wrap si me paso del borde de la pantalla horizontalmente
	if(wrap && x + (CHAR_WIDTH * font_size) > VBE_mode_info->width){
		x = 0;
		y += (CHAR_HEIGHT * font_size) + INTERLINE;
    }

    // si es el caracter de borrar, muevo el cursor para atrás, seteo el flag para después dejarlo quieto y cambio el caracter a espacio
    if(c == 8){
        is_deleting = 1;
        c = ' ';

        if(x == 0){
            x = VBE_mode_info->width - (CHAR_WIDTH * font_size);
            y -= (CHAR_HEIGHT * font_size) + INTERLINE;
        } else {
            x -= (CHAR_WIDTH * font_size);
        }
    }

	// Obtener el puntero al array de bytes del carácter
    uint8_t *character = font[c];
    if (character == 0) {
		character = unsupported_char;
    }

    // Dibuja el carácter usando los bits en elcharacter 
	for (uint64_t i = 0; i < CHAR_HEIGHT; i++) {
		for (uint64_t j = 0; j < CHAR_WIDTH; j++) {
			for (uint64_t k = 0; k < font_size; k++) {
				for (uint64_t l = 0; l < font_size; l++) {
					if (character[i] & (1 << (7 - j))) {
						putPixel(textColor, x + j * font_size + l, y + i * font_size + k);
					} else {
						putPixel(bgColor, x + j * font_size + l, y + i * font_size + k);
					}
				}
			}
		}
	}

	x += (CHAR_WIDTH * font_size) * !is_deleting; // si estoy borrando, no incremento x
}

void drawCharAt(char c, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	drawChar(c, textColor, bgColor, 0);
	x = oldX;
	y = oldY;
}

void drawString(char * string, uint32_t textColor, uint32_t bgColor) {
	while (*string) {
		drawChar(*string++, textColor, bgColor, 1);
	}
}

void drawStringAt(char * string, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	while (*string) {
		drawChar(*string++, textColor, bgColor, 0);
	}
	x = oldX;
	y = oldY;
}

void drawNumber(uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap){
	if(num == 0){
		drawChar('0', textColor, bgColor, wrap);
		return;
	}

	char buffer[50];
	int i = 0;
	while(num > 0){
		buffer[i] = num % 10 + '0';
		num /= 10;
		i++;
	}
	buffer[i] = 0;

	i--;
	while(i >= 0){
		drawChar(buffer[i], textColor, bgColor, wrap);
		i--;
	}
}

void drawNumberAt(uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	drawNumber(num, textColor, bgColor, 0);
	x = oldX;
	y = oldY;
}

void drawHex(uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap){
	if(num == 0){
		drawChar('0', textColor, bgColor, wrap);
		return;
	}

	char buffer[50];
	int i = 0;
	while(num > 0){
		buffer[i] = num % 16 < 10 ? num % 16 + '0' : num % 16 - 10 + 'A';
		num /= 16;
		i++;
	}
	buffer[i] = 0;

	i--;
	while(i >= 0){
		drawChar(buffer[i], textColor, bgColor, wrap);
		i--;
	}
}

void drawHexAt(uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	drawHex(num, textColor, bgColor, 0);
	x = oldX;
	y = oldY;
}

// imprime un número pero sin necesidad de especificar color ni nada (para debugging)
void simpleDrawNumber(uint64_t num){
	uint64_t textColor = 0xFFFFFF;
	uint64_t bgColor = 0x000000;
	
	if(num == 0){
		drawChar('0', textColor, bgColor, 1);
		return;
	}

	char buffer[50];
	int i = 0;
	while(num > 0){
		buffer[i] = num % 10 + '0';
		num /= 10;
		i++;
	}
	buffer[i] = 0;

	i--;
	while(i >= 0){
		drawChar(buffer[i], textColor, bgColor, 1);
		i--;
	}
}


// BITMAPS

// bitmap format is an array of 32 bit integers, each one representing a pixel color in hexa
// maybe it would be more efficent not to use putPixel, but to write directly to the framebuffer
void drawBitmap(uint32_t * bitmap, uint64_t width, uint64_t height, Point * position, uint32_t scale){
	if(scale < 1){
		return;
	}

	for(uint64_t i = 0; i < height; i++){
		for(uint64_t j = 0; j < width; j++){
			for(uint64_t k = 0; k < scale; k++){
				for(uint64_t l = 0; l < scale; l++){
					putPixel(bitmap[i * width + j], position->x + j * scale + k, position->y + i * scale + l);
				}
			}
		}
	}
}

// CURSOR

int isCursorInBoundaries(uint32_t line, uint32_t column){
	return line * (CHAR_HEIGHT * font_size) + line * INTERLINE < VBE_mode_info->height && column * (CHAR_WIDTH * font_size) < VBE_mode_info->width;
}

void setCursorLine(uint32_t line){
    y = line * (CHAR_HEIGHT * font_size) + line * INTERLINE;
}

void setCursorColumn(uint32_t column){
    x = column * (CHAR_WIDTH * font_size);
}

uint32_t getCursorLine(){
	return y / ((CHAR_HEIGHT * font_size) + INTERLINE);
}

uint32_t getCursorColumn(){
	return x / (CHAR_WIDTH * font_size);
}


uint64_t getScreenWidth(){
	return VBE_mode_info->width;
}
uint64_t getScreenHeight(){
	return VBE_mode_info->height;
}
uint64_t getCharWidth(){
	return (CHAR_WIDTH * font_size);
}
uint64_t getCharHeight(){
	return (CHAR_HEIGHT * font_size);
}

// GENERAL

void clearScreen(uint32_t bgColor){
	uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
	uint64_t offset = 0;

	uint8_t channel1 = (bgColor) & 0xFF;
	uint8_t channel2 = (bgColor >> 8) & 0xFF;
	uint8_t channel3 = (bgColor >> 16) & 0xFF;

    for(uint32_t i = 0; i < VBE_mode_info->height; i++){
		offset = (i * VBE_mode_info->pitch);

        for(uint32_t j = 0; j < VBE_mode_info->width; j++){
            // putPixel(bgColor, j, i);
			framebuffer[offset] = channel1;
			framebuffer[offset+1] = channel2;
			framebuffer[offset+2] = channel3;
			offset += 3;
        }
    }

    x = 0;
    y = 0;
}

void setFontSize(uint8_t fontSize){
	if(fontSize < 1 || fontSize > 8){
		return;
	}
	font_size = fontSize;
}

void setFont(Font newFont){
	font = newFont;
}