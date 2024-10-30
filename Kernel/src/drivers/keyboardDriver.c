#include <drivers/keyboardDriver.h>
#include <stdint.h>

static char pressed_keys[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


// Function to get the current pressed keys
// It returns a pointer to the array of pressed keys, so if the user wants to modify it, technically it can, 
// But it would be ignoring the const qualifier and shouldn't be done
// The function could recieve a pointer to an array as an argument and return there, but for simplicity I think this is fine (to-do?)
const char* get_pressed_keys() {
    return pressed_keys;
}

// TODO: implementar handleKeyPress para que la lógica de qué hacer con la tecla apretada no esté en la interrupción


// Set a key as pressed, returns 0 if successful, 1 if there are no empty slots
// (normally a keyboard does the same, if you press more keys than it can handle, it will ignore the extra ones)
const char set_key(char scan_code) {
    for(int i = 0; i < 6; i++) {
        if(pressed_keys[i] == 0x00) {
            pressed_keys[i] = scan_code;
            return 0;
        }
    }

    return 1;  // If there are no empty slots, return 1
}


const char release_key(char scan_code) {
    for(int i = 0; i < 6; i++) {
        if(pressed_keys[i] == scan_code) {
            pressed_keys[i] = 0x00;
            return 0;
        }
    }

    return 1;  // If the key was not found, return 1
}


// HELPERS (más que nada para trabajar con keycodes/ASCII)

// convierte de keycode a ascii
// char keycodeToAscii(char keycode) {
// 	switch (keycode) {
// 		case 0x0E:
// 			return ASCII_BS;
// 		case 0x1E:
// 			return 'a';
// 		case 0x30:
// 			return 'b';
// 		case 0x2E:
// 			return 'c';
// 		case 0x20:
// 			return 'd';
// 		case 0x12:
// 			return 'e';
// 		case 0x21:
// 			return 'f';
// 		case 0x22:
// 			return 'g';
// 		case 0x23:
// 			return 'h';
// 		case 0x17:
// 			return 'i';
// 		case 0x24:
// 			return 'j';
// 		case 0x25:
// 			return 'k';
// 		case 0x26:
// 			return 'l';
// 		case 0x32:
// 			return 'm';
// 		case 0x31:
// 			return 'n';
// 		case 0x18:
// 			return 'o';
// 		case 0x19:
// 			return 'p';
// 		case 0x10:
// 			return 'q';
// 		case 0x13:
// 			return 'r';
// 		case 0x1F:
// 			return 's';
// 		case 0x14:
// 			return 't';
// 		case 0x16:
// 			return 'u';
// 		case 0x2F:
// 			return 'v';
// 		case 0x11:
// 			return 'w';
// 		case 0x2D:
// 			return 'x';
// 		case 0x15:
// 			return 'y';
// 		case 0x2C:
// 			return 'z';
// 		case 0x02:
// 			return '1';
// 		case 0x03:
// 			return '2';
// 		case 0x04:
// 			return '3';
// 		case 0x05:
// 			return '4';
// 		case 0x06:
// 			return '5';
// 		case 0x07:
// 			return '6';
// 		case 0x08:
// 			return '7';
// 		case 0x09:
// 			return '8';
// 		case 0x0A:
// 			return '9';
// 		case 0x0B:
// 			return '0';
// 		case 0x39:
// 			return ' ';
// 		case 0x1C:
// 			return ASCII_LF;
// 		default:
// 			return ASCII_NUL; // si no es un caracter ascii, devuelvo NUL
// 	}
// }

// convierte de keycode a ASCII
char keycode_to_ascii[256] = {
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
    ASCII_CR,         // 0x1C (Enter)
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


char keycodeToAscii(char keycode) {
	return keycode_to_ascii[keycode];
}