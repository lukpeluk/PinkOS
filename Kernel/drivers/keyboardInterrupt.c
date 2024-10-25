// utiliza la función getKeyCode que se encuentra en el archivo getKeyCode.asm
// Setea la tecla como pressed o released según el scancode, usa el eventHandlerManager
// Además convierte el scancode a PinkMapping y lo envía a la función handle_key_press

#include <stdint.h>
#include <naiveConsole.h>
#include <videoDriver.h>
#include <eventHandlerManager.h>

extern char getKeyCode();

// convierte de keycode a ascii
char keycodeToAscii(char keycode){
	// convierte el keycode a ascii
	switch (keycode)
	{
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
			// return keycode);
			break;
	}
}

// Convierte KeyCodes a PinkMappings
// Devuelve -1 si no es un caracter válido (o si es un keycode de release y no de press)
int keycodeToPinkMap(char keycode){
	// PinkMapping (primera versión, provisoria) asigna del 0 al 9 a las teclas de 0 a 9 
	// y luego asigna las letras de la A a la Z en minúscula a las teclas de 10 a 35
	// con espacio en la 36 y enter en la 37

	// convierte el keycode a PinkMapping
	switch (keycode)
	{
		case 0x02:
			return 1;
			break;
		case 0x03:
			return 2;
			break;
		case 0x04:
			return 3;
			break;
		case 0x05:
			return 4;
			break;
		case 0x06:
			return 5;
			break;
		case 0x07:
			return 6;
			break;
		case 0x08:
			return 7;
			break;
		case 0x09:
			return 8;
			break;
		case 0x0A:
			return 9;
			break;
		case 0x0B:
			return 0;
			break;
		case 0x1E:
			return 10;
			break;
		case 0x30:
			return 11;
			break;
		case 0x2E:
			return 12;
			break;
		case 0x20:
			return 13;
			break;
		case 0x12:
			return 14;
			break;
		case 0x21:
			return 15;
			break;
		case 0x22:
			return 16;
			break;
		case 0x23:
			return 17;
			break;
		case 0x17:
			return 18;
			break;
		case 0x24:
			return 19;
			break;
		case 0x25:
			return 20;
			break;
		case 0x26:
			return 21;
			break;
		case 0x32:
			return 22;
			break;
		case 0x31:
			return 23;
			break;
		case 0x18:
			return 24;
			break;
		case 0x19:
			return 25;
			break;
		case 0x10:
			return 26;
			break;
		case 0x13:
			return 27;
			break;
		case 0x1F:
			return 28;
			break;
		case 0x14:
			return 29;
			break;
		case 0x16:
			return 30;
			break;
		case 0x2F:
			return 31;
			break;
		case 0x11:
			return 32;
			break;
		case 0x2D:
			return 33;
			break;
		case 0x15:
			return 34;
			break;
		case 0x2C:
			return 35;
			break;

		// espacio
		case 0x39:
			return 36;
			break;
		// enter
		case 0x1C:
			return 37;
			break;
		// backspace
		case 0x0E:
			return 38;
			break;

		default:
			// probablemente basura
			return -1;
			break;
	}
}

// assumes scan code set is 1
void int_21() {
	char c = getKeyCode();

	// checks whether the key was pressed or released
	// (released keys are 0x80 + the keycode of the pressed key)
	// TODO: que el handler reciba además del PinkMapping si la tecla se apretó o soltó
	// podría mandarle el scancode también por si le sirve, no estaría de más
	// ah, igual lo puede pedir del buffer de teclado con la syscall que implementaríamos
	if(c < 0x81){
		set_key(c);

		int pinkChar = keycodeToPinkMap(c);
		if(pinkChar != -1)
			handle_key_press(pinkChar);
	}
	else if (c < 0xD9) {
		release_key(c - 0x80);
	} else {
		// unsupported scancode
	}

		
}
