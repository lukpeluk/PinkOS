#include <drivers/keyboardDriver.h>
#include <drivers/videoDriver.h>  // just for debugging
#include <stdint.h>

extern char getKeyCode();

#define ADVANCE_INDEX(index) index = (index + 1) % 20
#define BUFFER_SIZE 10
#define PRESSED_KEYS_CACHE_SIZE 6

// otra opción sería un arreglo con un elemento por scan code, a modo de flag
// pero me parece demasiado siendo que los teclados habitualmente no mandan eventos de más de 6 teclas 
// volvería O(1) el seteo y el borrado de teclas, pero empeora el caso de listar las teclas apretadas
static char pressed_keys[PRESSED_KEYS_CACHE_SIZE] = {0x00};
static char pressed_keys_special_keycode_flag[PRESSED_KEYS_CACHE_SIZE] = {0x00};  // if a one, the key is a special keycode

static KeyboardEvent keyboardEventBuffer[BUFFER_SIZE];
int bufferReadIndex = 0;
int bufferWriteIndex = 0;

static int shift_pressed = 0;
static int altgr_pressed = 0;
static int caps_lock = 0;

static int handling_special_scancode = 0; // indicates if a scancode is one of the special ones, that use two interrupts

void addKeyboardEvent(char event_type, char ascii, char scan_code);
const char set_key(char scan_code, char is_special);
void release_key(char scan_code, char is_special);

// intended to be called by int_21, assumes scancode set 1
KeyboardEvent processKeyPress() {
	unsigned char c = getKeyCode();
    char ascii = 0;
    char event_type = 0;

    // debugging
    // drawChar('\n', 0xFFFFFF, 0);
    // drawHex((uint64_t)c, 0xFFFFFF, 0);
    // drawChar('\n', 0xFFFFFF, 0);
    

    if(c == 0xE0) {
        handling_special_scancode = 1;
        return (KeyboardEvent) {event_type, ascii, c};
    }

    if(handling_special_scancode) {
        handling_special_scancode = 0;
        event_type = c < 0x90 ? 3 : c < 0xE0 ? 4 : 0;
    } else {
        ascii = keycodeToAscii(c);
        event_type = c < 0x81 ? 1 : c < 0xD9 ? 2 : 0;
    }

	// checks whether the key was pressed or released (released keys are 0x80 + the keycode of the pressed key)
    // this is to update the pressed_keys array
	if(event_type == 1 || event_type == 3) {
        drawString("pressed\n", 0xFFFFFF, 0);
		set_key(c, event_type == 3);
	}
	else if (event_type == 2 || event_type == 4) {
        drawString("released\n", 0xFFFFFF, 0);
		release_key(c - 0x80, event_type == 4);
	} 

    KeyboardEvent event = {event_type, ascii, c};
    addKeyboardEvent(event_type, ascii, c);       // adds the event to the buffer
    return event;
}


int isKeyPressed(char scan_code, char is_special) {
    is_special = is_special ? 1 : 0;

    for(int i = 0; i < PRESSED_KEYS_CACHE_SIZE; i++) {
        if(pressed_keys_special_keycode_flag[i] == is_special && pressed_keys[i] == scan_code) {
            return 1;
        }
    }
    return 0;
}


// Dequeue a KeyboardEvent from the buffer, returns NULL if the buffer is empty
// returns as copy, so it cannot be given directly to the handler
KeyboardEvent getKeyboardEvent() {
    if (bufferReadIndex == bufferWriteIndex) {
        return (KeyboardEvent) {0, 0, 0};  // If the buffer is empty, return an empty event
    }
    KeyboardEvent event = keyboardEventBuffer[bufferReadIndex];
    ADVANCE_INDEX(bufferReadIndex);
    return event;
}

// Enqueue a KeyboardEvent to the buffer, if the buffer is full, the oldest event is overwritten
void addKeyboardEvent(char event_type, char ascii, char scan_code) {
    keyboardEventBuffer[bufferWriteIndex] = (KeyboardEvent) {event_type, ascii, scan_code};
    ADVANCE_INDEX(bufferWriteIndex);
    if(bufferReadIndex == bufferWriteIndex) {
        ADVANCE_INDEX(bufferReadIndex);
    }
}

void clearKeyboardBuffer() {
    bufferReadIndex = bufferWriteIndex;
}



// Set a key as pressed, returns 0 if successful, 1 if there are no empty slots
// (normally a keyboard does the same, if you press more keys than it can handle, it will ignore the extra ones)
const char set_key(char scan_code, char is_special) {
    is_special = is_special ? 1 : 0;

    // Flags for modifier keys
    if(!is_special){
        shift_pressed = scan_code == 0x2A ? 1 : shift_pressed;
        altgr_pressed = scan_code == 0x38 ? 1 : altgr_pressed;
        caps_lock = scan_code == 0x3A ? 1 : caps_lock;
    }

    int avaliable_slot = -1;
    for(int i = 0; i < PRESSED_KEYS_CACHE_SIZE; i++) {
        if(pressed_keys[i] == scan_code && pressed_keys_special_keycode_flag[i] == is_special) {
            return 0;  // If the key is already pressed, return 0
        }
        if(pressed_keys[i] == 0x00) {
            avaliable_slot = i;
        }
    }
    if (avaliable_slot != -1) {
        pressed_keys[avaliable_slot] = scan_code;
        pressed_keys_special_keycode_flag[avaliable_slot] = is_special;
        return 0;
    }

    return 1;  // If there are no empty slots, return 1
}


void release_key(char scan_code, char is_special) {
    is_special = is_special ? 1 : 0;

    // Flags for modifier keys
    if(!is_special){
        shift_pressed = scan_code == 0x2A ? 0 : shift_pressed;
        altgr_pressed = scan_code == 0x38 ? 0 : altgr_pressed;
        caps_lock = scan_code == 0x3A ? 0 : caps_lock;
    }

    for(int i = 0; i < PRESSED_KEYS_CACHE_SIZE; i++) {
        if(pressed_keys[i] == scan_code && pressed_keys_special_keycode_flag[i] == is_special) {
            pressed_keys[i] = 0x00;
        }
    }
}


// convierte de keycode a ASCII
char base_layer[256] = {
    ASCII_NUL,        // 0x00
    ASCII_ESC,        // 0x01
    '1',              // 0x02
    '2',              // 0x03
    '3',              // 0x04
    '4',              // 0x05
    '5',              // 0x06
    '6',              // 0x07
    '7',              // 0x08
    '8',              // 0x09
    '9',              // 0x0A
    '0',              // 0x0B
    '\'',             // 0x0C (Apostrofe)
    '¡',              // 0x0D (Signo de exclamación)
    ASCII_BS,         // 0x0E (Backspace)
    ASCII_HT,         // 0x0F (Tab)
    'q',              // 0x10
    'w',              // 0x11
    'e',              // 0x12
    'r',              // 0x13
    't',              // 0x14
    'y',              // 0x15
    'u',              // 0x16
    'i',              // 0x17
    'o',              // 0x18
    'p',              // 0x19
    '`',              // 0x1A (Acento grave)
    '+',              // 0x1B (Signo más)
    ASCII_LF,         // 0x1C (Enter)
    ASCII_NUL,        // 0x1D (Control izquierdo)
    'a',              // 0x1E
    's',              // 0x1F
    'd',              // 0x20
    'f',              // 0x21
    'g',              // 0x22
    'h',              // 0x23
    'j',              // 0x24
    'k',              // 0x25
    'l',              // 0x26
    'ñ',              // 0x27
    '{',              // 0x28
    '}',              // 0x29
    ASCII_NUL,        // 0x2A (Shift izquierdo)
    '|',              // 0x2B
    'z',              // 0x2C
    'x',              // 0x2D
    'c',              // 0x2E
    'v',              // 0x2F
    'b',              // 0x30
    'n',              // 0x31
    'm',              // 0x32
    ',',              // 0x33
    '.',              // 0x34
    '-',              // 0x35
    ASCII_NUL,        // 0x36 (Shift derecho)
    '*',              // 0x37 (Teclado numérico *)
    ASCII_NUL,        // 0x38 (Alt izquierdo)
    ' ',              // 0x39 (Espacio)
    ASCII_NUL,        // 0x3A (Bloq Mayús)
    ASCII_NUL,        // 0x3B (F1)
    ASCII_NUL,        // 0x3C (F2)
    ASCII_NUL,        // 0x3D (F3)
    ASCII_NUL,        // 0x3E (F4)
    ASCII_NUL,        // 0x3F (F5)
    ASCII_NUL,        // 0x40 (F6)
    ASCII_NUL,        // 0x41 (F7)
    ASCII_NUL,        // 0x42 (F8)
    ASCII_NUL,        // 0x43 (F9)
    ASCII_NUL,        // 0x44 (F10)
    ASCII_NUL,        // 0x45 (Bloq Num)
    ASCII_NUL,        // 0x46 (Bloq Despl)
    '7',              // 0x47 (Teclado numérico 7)
    '8',              // 0x48 (Teclado numérico 8)
    '9',              // 0x49 (Teclado numérico 9)
    '-',              // 0x4A (Teclado numérico -)
    '4',              // 0x4B (Teclado numérico 4)
    '5',              // 0x4C (Teclado numérico 5)
    '6',              // 0x4D (Teclado numérico 6)
    '+',              // 0x4E (Teclado numérico +)
    '1',              // 0x4F (Teclado numérico 1)
    '2',              // 0x50 (Teclado numérico 2)
    '3',              // 0x51 (Teclado numérico 3)
    '0',              // 0x52 (Teclado numérico 0)
    '.',              // 0x53 (Teclado numérico .)
    ASCII_NUL,        // 0x57 (F11)
    ASCII_NUL,        // 0x58 (F12)
    [0x59 ... 0xFF] = ASCII_NUL  // Relleno con ASCII_NUL
};

char shift_layer[256] = {
    ASCII_NUL, ASCII_ESC, '!', '"', '#', '$', '%', '&', '/', '(', ')', '=', '?', '¿',
    ASCII_BS, ASCII_HT, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '^', '*',
    ASCII_CR, ASCII_NUL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ñ', '[', ']',
    ASCII_NUL, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_', ASCII_NUL, '*',
    ASCII_NUL, ' ', ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL,
    ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.', ASCII_NUL, ASCII_NUL, [0x59 ... 0xFF] = ASCII_NUL
};

char caps_lock_layer[256] = {
    ASCII_NUL, ASCII_ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\'', '¡',
    ASCII_BS, ASCII_HT, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '+',
    ASCII_CR, ASCII_NUL, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ñ', '{', '}',
    ASCII_NUL, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '-', ASCII_NUL, '*',
    ASCII_NUL, ' ', ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL,
    ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, '7', '8', '9', '-', '4', '5',
    '6', '+', '1', '2', '3', '0', '.', ASCII_NUL, ASCII_NUL, [0x59 ... 0xFF] = ASCII_NUL
};

char altgr_layer[256] = {
    ASCII_NUL, ASCII_ESC, '|', '@', '#', '~', '€', '¬', '{', '[', ']', '}', '\\', ASCII_NUL,
    ASCII_BS, ASCII_HT, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL,
    ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_CR, ASCII_NUL, 
    ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, 
    ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, 
    ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, 
    ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, ASCII_NUL, '7', '8', '9', 
    '-', '4', '5', '6', '+', '1', '2', '3', '0', '.', ASCII_NUL, ASCII_NUL, [0x59 ... 0xFF] = ASCII_NUL
};


// keycode must be in the ascii range (TODO: fix)
// should handle both press and release keycodes
char keycodeToAsciiWIP(char keycode) {
    keycode = keycode < 0x81 ? keycode : keycode < 0xD9 ? keycode - 80 : 0;

    char ascii = ASCII_NUL;
    if (shift_pressed) {
        ascii = shift_layer[keycode];
    }
    else if (altgr_pressed) {
        ascii = altgr_layer[keycode];
    }
    else if (caps_lock) {
        ascii = caps_lock_layer[keycode];
    }
    if (ascii == ASCII_NUL) {
        ascii = base_layer[keycode];
    }
    return ascii;
}


// convierte de keycode a ascii (forma vieja)
// se usa temporalmente porque la otra no funca
char keycodeToAscii(char keycode) {
    keycode = keycode < 0x81 ? keycode : keycode < 0xD9 ? keycode - 80 : 0;
	if(!keycode) return 0;

	switch (keycode) {
		case 0x0E:
			return ASCII_BS;
		case 0x1E:
			return 'a';
		case 0x30:
			return 'b';
		case 0x2E:
			return 'c';
		case 0x20:
			return 'd';
		case 0x12:
			return 'e';
		case 0x21:
			return 'f';
		case 0x22:
			return 'g';
		case 0x23:
			return 'h';
		case 0x17:
			return 'i';
		case 0x24:
			return 'j';
		case 0x25:
			return 'k';
		case 0x26:
			return 'l';
		case 0x32:
			return 'm';
		case 0x31:
			return 'n';
		case 0x18:
			return 'o';
		case 0x19:
			return 'p';
		case 0x10:
			return 'q';
		case 0x13:
			return 'r';
		case 0x1F:
			return 's';
		case 0x14:
			return 't';
		case 0x16:
			return 'u';
		case 0x2F:
			return 'v';
		case 0x11:
			return 'w';
		case 0x2D:
			return 'x';
		case 0x15:
			return 'y';
		case 0x2C:
			return 'z';
		case 0x02:
			return '1';
		case 0x03:
			return '2';
		case 0x04:
			return '3';
		case 0x05:
			return '4';
		case 0x06:
			return '5';
		case 0x07:
			return '6';
		case 0x08:
			return '7';
		case 0x09:
			return '8';
		case 0x0A:
			return '9';
		case 0x0B:
			return '0';
		case 0x39:
			return ' ';
		case 0x1C:
			return ASCII_LF;
        case 0x01:
            return ASCII_ESC;

		default:
			return ASCII_NUL; // si no es un caracter ascii, devuelvo NUL
	}
}


