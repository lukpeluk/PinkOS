#include <drivers/videoDriver.h>
#include <drivers/pitDriver.h>

#include <drivers/defaultFont.h>
#include <memoryManager/memoryManager.h>
#include <windowManager/windowManager.h>
#include <lib.h>
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
static int font_size = 2;
#define FONT_SIZE_LIMIT 6

#define FRAME_RATE 15 // frames per second

static int last_frame_time = 0; // last time the video buffer was updated, in milliseconds
static uint8_t * staging_buffer = NULL;

void initVideoDriver() {
	staging_buffer = createVideoBuffer();

	if(staging_buffer == NULL) {
		log_to_serial("initVideoDriver: Error al allocar memoria para el staging buffer");
		return;
	}
}

void overlayTest(uint8_t * overlay_buffer) {
	static Point position = {0, 0}; // Cursor position in the video buffer
	position.x += 5; // Reset position to the start of the buffer
	position.y += 5;

	if(position.x >= VBE_mode_info->width) {
		position.x = 0;
		position.y = 0;
	}

	// Vacío el overlay buffer y escribo el mensajito de aprecio a francis
	memset(overlay_buffer, 0, VBE_mode_info->width * VBE_mode_info->height * (VBE_mode_info->bpp / 8));
	drawStringAt(overlay_buffer, "Francis te AMO <3", 0xFF0000, 0x00000000, &position);
}


void videoLoop() {
	// This function is intended to be called every timer tick to update the video buffer.
	// Currently, it does nothing as the video buffer is updated directly when drawing.
	if (milliseconds_elapsed() - last_frame_time < (1000 / FRAME_RATE)) {
		return; // Not enough time has passed since the last frame, skip this update
	}
	uint8_t * focused_buffer = getFocusedBuffer();
	uint8_t * overlay_buffer = getOverlayBuffer();

	if(focused_buffer == NULL || overlay_buffer == NULL || staging_buffer == NULL) {
		log_to_serial("E: videoLoop: No focused buffer or staging buffer available");
		console_log("Values of the pointers: focused_buffer: %p, overlay_buffer: %p, staging_buffer: %p", focused_buffer, overlay_buffer, staging_buffer);
		return; // No focused window, nothing to do
	}

	overlayTest(overlay_buffer);
	
	last_frame_time = milliseconds_elapsed();
	lightspeed_memcpy(staging_buffer, focused_buffer, VBE_mode_info->width * VBE_mode_info->height * (VBE_mode_info->bpp / 8));

	// Copia el overlay buffer pero ignora los píxeles negros (0x00000000), que son tomados como transparentes
	// TODO: ver si esto puede tener problemas al usar 24 bytes por píxel (al compararlo casteado a 32) parece funcar bien igual
	for(uint64_t i = 0; i < VBE_mode_info->width * VBE_mode_info->height * (VBE_mode_info->bpp / 8); i += (VBE_mode_info->bpp / 8)) {
		uint32_t pixel = *(uint32_t *)(overlay_buffer + i);
		if(pixel != 0x00000000) {
			*(uint32_t *)(staging_buffer + i) = pixel;
		}
	}

	lightspeed_memcpy((void*)VBE_mode_info->framebuffer, staging_buffer, VBE_mode_info->width * VBE_mode_info->height * (VBE_mode_info->bpp / 8));
}

uint8_t * createVideoBuffer() {
	uint8_t * buffer = malloc(VBE_mode_info->width * VBE_mode_info->height * (VBE_mode_info->bpp / 8));
	console_log("Creating video buffer with %d bytes per pixel", (int)(VBE_mode_info->bpp / 8));

	if(buffer == NULL) {
		log_to_serial("E: createVideoBuffer: Failed to allocate memory for video buffer");
		return NULL; // Memory allocation failed
	}
	// Clear the buffer to black
	memset(buffer, 0, VBE_mode_info->width * VBE_mode_info->height * (VBE_mode_info->bpp / 8));
	return buffer;
}


void putPixel(uint8_t * videoBuffer, uint32_t hexColor, uint64_t x, uint64_t y) {
    if(x >= VBE_mode_info->width || y >= VBE_mode_info->height || videoBuffer == NULL) {
        return;
    }
    uint8_t * framebuffer = (uint8_t *)(uintptr_t) videoBuffer;
    uint64_t offset = (x * ((VBE_mode_info->bpp)/8)) + (y * VBE_mode_info->pitch);
    framebuffer[offset]     =  (hexColor) & 0xFF;
    framebuffer[offset+1]   =  (hexColor >> 8) & 0xFF; 
    framebuffer[offset+2]   =  (hexColor >> 16) & 0xFF;
}

// BASIC SHAPES

void drawRectangle(uint8_t * videoBuffer, Point * start, Point * end, uint32_t hexColor){
	if(videoBuffer == NULL || start == NULL || end == NULL) {
		return; // Invalid parameters
	}
	if(start->x >= end->x || start->y >= end->y || start->x >= VBE_mode_info->width || start->y >= VBE_mode_info->height || end->x > VBE_mode_info->width || end->y > VBE_mode_info->height) {
		return; // Invalid rectangle dimensions
	}
	if(start->x < 0 || start->y < 0 || end->x < 0 || end->y < 0) {
		return; // Negative coordinates are not allowed
	}
	for(uint64_t i = start->x; i < end->x; i++){
		for(uint64_t j = start->y; j < end->y; j++){
			putPixel(videoBuffer, hexColor, i, j);
		}
	}
}

void drawRectangleBoder(uint8_t * videoBuffer, Point * start, Point * end, uint32_t thickness, uint32_t hexColor){
	for(uint64_t i = start->x; i < end->x; i++){
		for(uint64_t j = start->y; j < end->y; j++){
			if(i < start->x + thickness || i >= end->x - thickness || j < start->y + thickness || j >= end->y - thickness){
				putPixel(videoBuffer, hexColor, i, j);
			}
		}
	}
}


#define CHAR_WIDTH 8
#define CHAR_HEIGHT 8
#define FONT_NOF_CHARS 95

static uint64_t x = 0;
static uint64_t y = 0;
static char INTERLINE = 4;

// funca con caracteres imprimibles soportados por la tipografía, y con el salto de línea y delete
// hace wrapping automático, podría configurarse con un flag
// para borrar le escribe un espacio en blanco por arriba
void drawChar(uint8_t * videoBuffer, char c, uint32_t textColor, uint32_t bgColor, int wrap) {
    int is_deleting = 0;

	// si el caracter es un espacio, no lo dibuja

	// salto de línea (antes del wrapping pues este no le debe afectar)
	if(c == '\n'){
        x = 0;
		y += (CHAR_HEIGHT * font_size) + INTERLINE;
		return;
	}

    // no tiene sentido un wrapping vertical, así que si se pasa del borde de la pantalla para abajo, no imprime nada
    if(y + (CHAR_HEIGHT * font_size) > VBE_mode_info->height){
        return;
    }
    
	// hago wrap si me paso del borde de la pantalla horizontalmente
	if(wrap && x + (CHAR_WIDTH * font_size) > VBE_mode_info->width){
		x = 0;
		y += (CHAR_HEIGHT * font_size) + INTERLINE;
    }

    // si es el caracter de borrar, muevo el cursor para atrás, seteo el flag para después dejarlo quieto y cambio el caracter a espacio
    if(c == '\b'){
        is_deleting = 1;
        c = ' ';

        if(x == 0){
            x = VBE_mode_info->width - (CHAR_WIDTH * font_size);
            y -= (CHAR_HEIGHT * font_size) + INTERLINE;
        } else {
            x -= (CHAR_WIDTH * font_size);
        }
    }else if(c == '\t'){
		x += (4 * CHAR_WIDTH * font_size);
	}

	if(!IS_PRINTABLE_CHAR(c)) return;

	// Obtener el puntero al array de bytes del carácter
    uint8_t *character = font[(unsigned char) c];
    if (character == 0) {
		character = unsupported_char;
    }

    // Dibuja el carácter usando los bits en elcharacter 
	for (uint64_t i = 0; i < CHAR_HEIGHT; i++) {
		for (uint64_t j = 0; j < CHAR_WIDTH; j++) {
			for (uint64_t k = 0; k < font_size; k++) {
				for (uint64_t l = 0; l < font_size; l++) {
					if (character[i] & (1 << (7 - j))) {
						putPixel(videoBuffer, textColor, x + j * font_size + l, y + i * font_size + k);
					} else {
						putPixel(videoBuffer, bgColor, x + j * font_size + l, y + i * font_size + k);
					}
				}
			}
		}
	}

	x += (CHAR_WIDTH * font_size) * !is_deleting; // si estoy borrando, no incremento x
}

void drawCharAt(uint8_t * videoBuffer, char c, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	drawChar(videoBuffer, c, textColor, bgColor, 0);
	x = oldX;
	y = oldY;
}

void drawString(uint8_t * videoBuffer, char * string, uint32_t textColor, uint32_t bgColor) {
	while (*string) {
		drawChar(videoBuffer, *string++, textColor, bgColor, 1);
	}
}

void drawStringAt(uint8_t * videoBuffer, char * string, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	while (*string) {
		drawChar(videoBuffer, *string++, textColor, bgColor, 0);
	}
	x = oldX;
	y = oldY;
}

void drawNumber(uint8_t * videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap){
	if(num == 0){
		drawChar(videoBuffer, '0', textColor, bgColor, wrap);
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
		drawChar(videoBuffer, buffer[i], textColor, bgColor, wrap);
		i--;
	}
}

void drawNumberAt(uint8_t * videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	drawNumber(videoBuffer, num, textColor, bgColor, 0);
	x = oldX;
	y = oldY;
}

void drawHex(uint8_t * videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, int wrap){
	if(num == 0){
		drawChar(videoBuffer, '0', textColor, bgColor, wrap);
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
		drawChar(videoBuffer, buffer[i], textColor, bgColor, wrap);
		i--;
	}
}

void drawHexAt(uint8_t * videoBuffer, uint64_t num, uint32_t textColor, uint32_t bgColor, Point * position){
	uint64_t oldX = x;
	uint64_t oldY = y;
	x = position->x;
	y = position->y;
	drawHex(videoBuffer, num, textColor, bgColor, 0);
	x = oldX;
	y = oldY;
}

// imprime un número pero sin necesidad de especificar color ni nada (para debugging)
void simpleDrawNumber(uint8_t * videoBuffer, uint64_t num){
	uint64_t textColor = 0xFFFFFF;
	uint64_t bgColor = 0x000000;
	
	if(num == 0){
		drawChar(videoBuffer, '0', textColor, bgColor, 1);
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
		drawChar(videoBuffer, buffer[i], textColor, bgColor, 1);
		i--;
	}
}


// BITMAPS

// bitmap format is an array of 32 bit integers, each one representing a pixel color in hexa
// maybe it would be more efficent not to use putPixel, but to write directly to the framebuffer
void drawBitmap(uint8_t * videoBuffer, uint32_t * bitmap, uint64_t width, uint64_t height, Point * position, uint32_t scale){
	if(scale < 1){
		return;
	}

	for(uint64_t i = 0; i < height; i++){
		for(uint64_t j = 0; j < width; j++){
			for(uint64_t k = 0; k < scale; k++){
				for(uint64_t l = 0; l < scale; l++){
					putPixel(videoBuffer, bitmap[i * width + j], position->x + j * scale + k, position->y + i * scale + l);
				}
			}
		}
	}
}

// CURSOR

int isCursorInBoundaries(uint32_t line, uint32_t column){
	return (column + 1) * (CHAR_WIDTH * font_size) >= VBE_mode_info->width ? (line + 2) * (CHAR_HEIGHT * font_size + INTERLINE) < VBE_mode_info->height : (line + 1) * (CHAR_HEIGHT * font_size + INTERLINE) < VBE_mode_info->height;
}

void setCursorLine(uint32_t line){
    y = line * (CHAR_HEIGHT * font_size + INTERLINE);
}

void setCursorColumn(uint32_t column){
    x = column * (CHAR_WIDTH * font_size);
}

uint32_t getCursorLine(){
	return y / (CHAR_HEIGHT * font_size + INTERLINE);
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

void clearScreen(uint8_t * videoBuffer, uint32_t bgColor){
    uint8_t * framebuffer = (uint8_t *)(uintptr_t) videoBuffer;
	if(framebuffer == NULL)
		return; // No framebuffer to clear

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
	if(fontSize < 1 || fontSize > FONT_SIZE_LIMIT){
		return;
	}
	font_size = fontSize;
}

void incFontSize(){
	if(font_size < FONT_SIZE_LIMIT){
		font_size++;
	}
}

void decFontSize(){
	if(font_size > 1){
		font_size--;
	}
}

void setFont(Font newFont){
	font = newFont;
}