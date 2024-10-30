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
char keycodeToAscii(char keycode){
	// convierte el keycode a ascii
	switch (keycode)
	{
		case 0x0E:
			return 8;
			break;
		case 0x1E:
			return 'a';
			break;
		case 0x30: 
			return 'b';
			break; 
		case 0x2E:
			return 'c';
			break;
		case 0x20:
			return 'd';
			break;
		case 0x12:
			return 'e';
			break;
		case 0x21:
			return 'f';
			break;
		case 0x22:
			return 'g';
			break;
		case 0x23:
			return 'h';
			break;
		case 0x17:
			return 'i';
			break;
		case 0x24:
			return 'j';
			break;
		case 0x25:
			return 'k';
			break;
		case 0x26:
			return 'l';
			break;
		case 0x32:
			return 'm';
			break;
		case 0x31:
			return 'n';
			break;
		case 0x18:
			return 'o';
			break;
		case 0x19:
			return 'p';
			break;
		case 0x10:
			return 'q';
			break;
		case 0x13:
			return 'r';
			break;
		case 0x1F:
			return 's';
			break;
		case 0x14:
			return 't';
			break;
		case 0x16:
			return 'u';
			break;
		case 0x2F:
			return 'v';
			break;
		case 0x11:
			return 'w';
			break;
		case 0x2D:
			return 'x';
			break;
		case 0x15:
			return 'y';
			break;
		case 0x2C:
			return 'z';
			break;
		case 0x02:
			return '1';
			break;
		case 0x03:
			return '2';
			break;
		case 0x04:
			return '3';
			break;
		case 0x05:
			return '4';
			break;
		case 0x06:
			return '5';
			break;
		case 0x07:
			return '6';
			break;
		case 0x08:
			return '7';
			break;
		case 0x09:
			return '8';
			break;
		case 0x0A:
			return '9';
			break;
		case 0x0B:
			return '0';
			break;
        // espacio en blanco
        case 0x39:
            return ' ';
            break;
		case 0x1C:
			return '\n';
			break;
		default:
			return 0; // si no es un caracter ascii, devuelvo 0
			break;
	}
}
