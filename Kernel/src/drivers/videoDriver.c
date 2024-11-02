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
typedef uint8_t * Font;

VBEInfoPtr VBE_mode_info = (VBEInfoPtr) 0x0000000000005C00;
static Font font = ibm_bios_font;

void putPixel(uint32_t hexColor, uint64_t x, uint64_t y) {
	uint8_t * framebuffer = (uint8_t *) VBE_mode_info->framebuffer;
	uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
	framebuffer[offset]     =  (hexColor) & 0xFF;
	framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
	framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}



uint8_t font2[2][16] = {
	// Character ' ' (space)
	{
		0b11111111, 
		0b10000001, 
		0b10000000, 
		0b10000000,
		0b10000000, 
		0b10000000, 
		0b10000000, 
		0b10000000,
		0b10000000, 
		0b10000000, 
		0b10000000, 
		0b10000000,
		0b10000000, 
		0b10000000, 
		0b10000001, 
		0b11111111
	},
	// Character 'A'
	{
		0b00011000, 0b00111100, 0b01100110, 0b11000011,
		0b11000011, 0b11111111, 0b11000011, 0b11000011,
		0b11000011, 0b11000011, 0b11000011, 0b00000000,
		0b00000000, 0b00000000, 0b00000000, 0b00000000
	},
	// Add more characters as needed...
};

uint8_t font_test_photoshop[][16] = {
	// Character 'test-a'
	{ 0b00000000, 0b00010100, 0b00011000, 0b00011000, 0b00111000, 0b00111100, 0b01111100, 0b01101100, 0b01101110, 0b11111111, 0b11111111, 0b11000111, 0b11000011, 0b11000011, 0b10000000, 0b00000000 },
	// Character 'test-b'
	{ 0b00000000, 0b00000000, 0b01100000, 0b01100000, 0b01100000, 0b01100000, 0b01100000, 0b01110000, 0b01111110, 0b01111110, 0b01100110, 0b01100111, 0b01111110, 0b01111100, 0b00000000, 0b00000000 },
};

// Pink Panther font
uint8_t pink_panther_font[][16] = {
	// Character 'K'
	{ 0b00001111, 0b11100011, 0b10000100, 0b10000100, 0b10001000, 0b10011000, 0b10110000, 0b11100000, 0b11110000, 0b10110000, 0b10011000, 0b10011000, 0b10001100, 0b10001110, 0b10000111, 0b11100000 },
	// Character 'J'
	{ 0b00111111, 0b00001100, 0b00001100, 0b00001100, 0b00001100, 0b00000100, 0b00001100, 0b00001100, 0b00001100, 0b00000100, 0b00001110, 0b10001100, 0b10001100, 0b11001100, 0b10110000, 0b00000000 },
	// Character 'H'
	{ 0b00011111, 0b11000110, 0b10000110, 0b10000110, 0b10000110, 0b10000110, 0b10000110, 0b10000110, 0b10000110, 0b11111110, 0b10000110, 0b10000110, 0b10000110, 0b10000110, 0b10001111, 0b11100000 },
	// Character 'I'
	{ 0b01111100, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b00110000, 0b11111000 },
	// Character 'Z'
	{ 0b11111111, 0b11000110, 0b11000110, 0b01001100, 0b10001100, 0b10011000, 0b00011000, 0b00110001, 0b00110001, 0b01100010, 0b01100011, 0b11000011, 0b11000011, 0b11111110, 0b00000000, 0b00000000 },
	// Character 'M'
	{ 0b00000011, 0b10000011, 0b11000011, 0b11000111, 0b11000111, 0b11100101, 0b11100101, 0b01101011, 0b10111011, 0b10111001, 0b10110011, 0b10011011, 0b10010001, 0b10000011, 0b10001111, 0b11000000 },
	// Character 'L'
	{ 0b11111000, 0b01100000, 0b01100000, 0b01100000, 0b01100000, 0b01100000, 0b01100000, 0b01100001, 0b01100001, 0b01100010, 0b01100011, 0b01100011, 0b01100011, 0b11101111, 0b00101000, 0b00000000 },
	// Character 'Y'
	{ 0b00001111, 0b11100010, 0b11000010, 0b01100100, 0b01100100, 0b01100100, 0b00111000, 0b00111000, 0b00010000, 0b00011000, 0b00010000, 0b00011000, 0b00011000, 0b00111000, 0b11111100, 0b00000000 },
	// Character 'N'
	{ 0b10001111, 0b11100010, 0b01100010, 0b01100010, 0b10110010, 0b01010010, 0b00110010, 0b10011000, 0b01001010, 0b10001110, 0b01001110, 0b10000110, 0b10000110, 0b10000110, 0b11000010, 0b11100010 },
	// Character 'O'
	{ 0b00010100, 0b00100010, 0b01100010, 0b01000011, 0b11000011, 0b11000011, 0b11000011, 0b11000001, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b01000010, 0b01000110, 0b00111000 },
	// Character 'X'
	{ 0b00001111, 0b11110010, 0b11000110, 0b01100100, 0b01100100, 0b00111000, 0b00111000, 0b00010000, 0b00011000, 0b00111000, 0b00101100, 0b01001100, 0b01000110, 0b10000110, 0b11001111, 0b11100000 },
	// Character 'U'
	{ 0b00001111, 0b11110010, 0b11000010, 0b10000010, 0b11000010, 0b10000010, 0b11000010, 0b10000010, 0b11000010, 0b10000010, 0b10000010, 0b11000010, 0b10000010, 0b11000100, 0b01000100, 0b00111000 },
	// Character 'B'
	{ 0b11111110, 0b01100011, 0b01100011, 0b01100001, 0b01100011, 0b01100001, 0b00110011, 0b01111110, 0b00100011, 0b01100011, 0b00100001, 0b00110011, 0b01100011, 0b11111100, 0b00000000, 0b00000000 },
	// Character 'C'
	{ 0b00110010, 0b01000110, 0b01000110, 0b11000010, 0b11000010, 0b11000010, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000001, 0b11000010, 0b01000010, 0b01100110, 0b00011010 },
	// Character 'T'
	{ 0b11111111, 0b10011011, 0b10011001, 0b00111001, 0b00010001, 0b00011001, 0b00011000, 0b00110000, 0b00011000, 0b00010000, 0b00011000, 0b00010000, 0b00010000, 0b00111000, 0b01010100, 0b00000000 },
	// Character 'V'
	{ 0b01001111, 0b11000010, 0b10000100, 0b10000100, 0b10000100, 0b11000100, 0b11001000, 0b01001000, 0b01101000, 0b01100000, 0b01110000, 0b01110000, 0b00100000, 0b00100000, 0b00100000, 0b00100000 },
	// Character 'A'
	{ 0b11111000, 0b00111000, 0b00011100, 0b00111100, 0b00101100, 0b00101100, 0b01000110, 0b01000110, 0b01000110, 0b01111110, 0b01000011, 0b10000011, 0b11000011, 0b10001111, 0b11100000, 0b00000000 },
	// Character 'W'
	{ 0b00000011, 0b10010000, 0b00010001, 0b00011000, 0b00111001, 0b00011001, 0b00101001, 0b10101101, 0b10101101, 0b11001110, 0b11000110, 0b11000110, 0b11000110, 0b11000100, 0b01000100, 0b00000000 },
	// Character 'D'
	{ 0b11111000, 0b11000110, 0b11000110, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b01000011, 0b11000011, 0b01000011, 0b11000110, 0b01000110, 0b11001100, 0b11110000, 0b00000000 },
	// Character 'S'
	{ 0b00111010, 0b01000110, 0b11000110, 0b10000010, 0b11000010, 0b10000010, 0b11000010, 0b11100000, 0b01110000, 0b00011000, 0b00001110, 0b00000110, 0b10000110, 0b10000110, 0b11000100, 0b10111000 },
	// Character 'R'
	{ 0b11111000, 0b11001100, 0b11000110, 0b10000110, 0b11000110, 0b11000110, 0b10000110, 0b11000110, 0b10001100, 0b11111000, 0b11010000, 0b10011000, 0b11011000, 0b10001100, 0b10000110, 0b11100101 },
	// Character 'E'
	{ 0b11111111, 0b01100011, 0b01110001, 0b00100001, 0b01110001, 0b00100000, 0b01110100, 0b00110100, 0b01101100, 0b00100101, 0b00110001, 0b00100001, 0b01100001, 0b11111111, 0b00000000, 0b00000000 },
	// Character 'G'
	{ 0b00110010, 0b11001110, 0b10000110, 0b10000100, 0b10000010, 0b10000000, 0b10000000, 0b10001010, 0b10011110, 0b10000110, 0b10000110, 0b10000110, 0b10000110, 0b11001110, 0b00101010, 0b00000000 },
	// Character 'P'
	{ 0b11111000, 0b11000110, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000111, 0b11000110, 0b11111000, 0b11000000, 0b11000000, 0b01000000, 0b11000000, 0b11000000, 0b11110000 },
	// Character 'Q'
	{ 0b00011000, 0b01100100, 0b01000010, 0b11000010, 0b11000011, 0b11000011, 0b10000011, 0b11000011, 0b10000011, 0b11000011, 0b11000011, 0b11000011, 0b11000010, 0b01000110, 0b01101100, 0b00111100 },
	// Character 'F'
	{ 0b11111111, 0b01100011, 0b01110001, 0b01100001, 0b00110001, 0b01100001, 0b00110100, 0b01100100, 0b00111110, 0b00100100, 0b01100010, 0b00100000, 0b00110000, 0b00100000, 0b01110000, 0b10101000 },
};


#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define FONT_NOF_CHARS 95

static uint8_t * currentVideo = (uint8_t*)0xB8000;
static uint64_t x = 0;
static uint64_t y = 0;
static char INTERLINE = 3;

// CÓDIGO DE MIERDA, PORFA REFACTORIZAR 

// TODO: que si un char no está soportado por la fuente, use un �

// funca con caracteres imprimibles soportados por la tipografía, y con el salto de línea y delete
// hace wrapping automático, podría configurarse con un flag
// para borrar le escribe un espacio en blanco por arriba
void drawChar(unsigned char c, uint32_t textColor, uint32_t bgColor) {
    int is_deleting = 0;

	// salto de línea (antes del wrapping pues este no le debe afectar)
	if(c == '\n'){
        x = 0;
		y += CHAR_HEIGHT + INTERLINE;
		return;
	}

    // no tiene sentido un wrapping vertical, así que si se pasa del borde de la pantalla para abajo, no imprime nada
    if(y > VBE_mode_info->height){
        return;
    }
    
	// hago wrap si me paso del borde de la pantalla horizontalmente
	if(x + CHAR_WIDTH > VBE_mode_info->width){
		x = 0;
		y += CHAR_HEIGHT + INTERLINE;
    }

    // si es el caracter de borrar, muevo el cursor para atrás, seteo el flag para después dejarlo quieto y cambio el caracter a espacio
    if(c == 8){
        is_deleting = 1;
        c = ' ';

        if(x == 0){
            x = VBE_mode_info->width - CHAR_WIDTH;
            y -= CHAR_HEIGHT + INTERLINE;
        } else {
            x -= CHAR_WIDTH;
        }
    }

	// Obtener el puntero al array de bytes del carácter
    uint8_t *bitmap = ibm_bios_font[c];
    if (bitmap == 0) {
		bitmap = unsupported_char;
    }

    // Dibuja el carácter usando los bits en el bitmap
    for (uint64_t i = 0; i < CHAR_HEIGHT; i++) {
        for (uint64_t j = 0; j < CHAR_WIDTH; j++) {
            if (bitmap[i] & (1 << (7 - j))) {
                putPixel(textColor, x + j, y + i);
            } else {
                putPixel(bgColor, x + j, y + i);
            }
        }
    }

	x += CHAR_WIDTH * !is_deleting; // si estoy borrando, no incremento x
}

// imprime un número
void drawNumber(uint64_t num, uint32_t textColor, uint32_t bgColor){
	if(num == 0){
		drawChar('0', textColor, bgColor);
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
		drawChar(buffer[i], textColor, bgColor);
		i--;
	}
}

void drawHex(uint64_t num, uint32_t textColor, uint32_t bgColor){
	if(num == 0){
		drawChar('0', textColor, bgColor);
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
		drawChar(buffer[i], textColor, bgColor);
		i--;
	}
}

// imprime un número
void simpleDrawNumber(uint64_t num){
	uint64_t textColor = 0xFFFFFF;
	uint64_t bgColor = 0x000000;
	
	if(num == 0){
		drawChar('0', textColor, bgColor);
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
		drawChar(buffer[i], textColor, bgColor);
		i--;
	}
}


void setCursorLine(uint32_t line){
    y = line * CHAR_HEIGHT + line * INTERLINE;
}

void setCursorColumn(uint32_t column){
    x = column * CHAR_WIDTH;
}

uint32_t getCursorLine(){
	return y / CHAR_HEIGHT;
}

uint32_t getCursorColumn(){
	return x / CHAR_WIDTH;
}

void drawString(char * string, uint32_t textColor, uint32_t bgColor) {
	while (*string) {
		drawChar(*string++, textColor, bgColor);
	}
}

// GRAPHIC MODE

void drawRectangle(Point start, Point end, uint32_t hexColor){
	for(uint64_t i = start.x; i < end.x; i++){
		for(uint64_t j = start.y; j < end.y; j++){
			putPixel(hexColor, i, j);
		}
	}
}

void drawCharAt(char c, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	drawChar(c, textColor, bgColor);
	x = oldX;
	y = oldY;
}

void drawStringAt(char * string, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	while (*string) {
		drawChar(*string++, textColor, bgColor);
	}
	x = oldX;
	y = oldY;
}


void clearScreen(uint32_t bgColor){
    for(uint32_t i = 0; i < VBE_mode_info->height; i++){
        for(uint32_t j = 0; j < VBE_mode_info->width; j++){
            putPixel(bgColor, j, i);
        }
    }
    x = 0;
    y = 0;
}

/*
   void printString(const char *str, uint32_t hexColor, uint64_t x, uint64_t y) {
   while (*str) {
   drawChar(*str++, hexColor, x, y);
   x += CHAR_WIDTH;
   }
   }
   */
