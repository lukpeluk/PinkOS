// código C que se encarga de leer el scancode del teclado y mostrarlo en pantalla.
// utiliza la función getKeyCode que se encuentra en el archivo getKeyCode.asm

#include <stdint.h>
#include <naiveConsole.h>

extern char getKeyCode();

// convierte de keycode a ascii
void keycodeToAscii(char keycode){
	// convierte el keycode a ascii
	switch (keycode)
	{
		case 0x1E:
			ncPrintChar('a');
			break;
		case 0x30: 
			ncPrintChar('b');
			break; 
		case 0x2E:
			ncPrintChar('c');
			break;
		case 0x20:
			ncPrintChar('d');
			break;
		case 0x12:
			ncPrintChar('e');
			break;
		case 0x21:
			ncPrintChar('f');
			break;
		case 0x22:
			ncPrintChar('g');
			break;
		case 0x23:
			ncPrintChar('h');
			break;
		case 0x17:
			ncPrintChar('i');
			break;
		case 0x24:
			ncPrintChar('j');
			break;
		case 0x25:
			ncPrintChar('k');
			break;
		case 0x26:
			ncPrintChar('l');
			break;
		case 0x32:
			ncPrintChar('m');
			break;
		case 0x31:
			ncPrintChar('n');
			break;
		case 0x18:
			ncPrintChar('o');
			break;
		case 0x19:
			ncPrintChar('p');
			break;
		case 0x10:
			ncPrintChar('q');
			break;
		case 0x13:
			ncPrintChar('r');
			break;
		case 0x1F:
			ncPrintChar('s');
			break;
		case 0x14:
			ncPrintChar('t');
			break;
		case 0x16:
			ncPrintChar('u');
			break;
		case 0x2F:
			ncPrintChar('v');
			break;
		case 0x11:
			ncPrintChar('w');
			break;
		case 0x2D:
			ncPrintChar('x');
			break;
		case 0x15:
			ncPrintChar('y');
			break;
		case 0x2C:
			ncPrintChar('z');
			break;
		case 0x02:
			ncPrintChar('1');
			break;
		case 0x03:
			ncPrintChar('2');
			break;
		case 0x04:
			ncPrintChar('3');
			break;
		case 0x05:
			ncPrintChar('4');
			break;
		case 0x06:
			ncPrintChar('5');
			break;
		case 0x07:
			ncPrintChar('6');
			break;
		case 0x08:
			ncPrintChar('7');
			break;
		case 0x09:
			ncPrintChar('8');
			break;
		case 0x0A:
			ncPrintChar('9');
			break;
		case 0x0B:
			ncPrintChar('0');
			break;
        // espacio en blanco
        case 0x39:
            ncPrintChar(' ');
            break;
		case 0x1C:
			ncPrintChar('\n');
			break;
		default:
			// ncPrintChar(keycode);
			break;
	}
}


void int_21() {
	char c = getKeyCode();
	keycodeToAscii(c);
}
