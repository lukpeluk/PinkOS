// código C que se encarga de leer el scancode del teclado y mostrarlo en pantalla.
// utiliza la función getKeyCode que se encuentra en el archivo getKeyCode.asm

#include <stdint.h>
#include <naiveConsole.h>
#include <videoDriver.h>

extern char getKeyCode();

// convierte de keycode a ascii
void keycodeToAscii(char keycode){
	// convierte el keycode a ascii
	switch (keycode)
	{
		case 0x1E:
			drawChar('a', 0x00df8090);
			break;
		case 0x30: 
			drawChar('b', 0x00df8090);
			break; 
		case 0x2E:
			drawChar('c', 0x00df8090);
			break;
		case 0x20:
			drawChar('d', 0x00df8090);
			break;
		case 0x12:
			drawChar('e',  0x00df8090);
			break;
		case 0x21:
			drawChar('f',  0x00df8090);
			break;
		case 0x22:
			drawChar('g',  0x00df8090);
			break;
		case 0x23:
			drawChar('h',  0x00df8090);
			break;
		case 0x17:
			drawChar('i',  0x00df8090);
			break;
		case 0x24:
			drawChar('j',  0x00df8090);
			break;
		case 0x25:
			drawChar('k',  0x00df8090);
			break;
		case 0x26:
			drawChar('l',  0x00df8090);
			break;
		case 0x32:
			drawChar('m',  0x00df8090);
			break;
		case 0x31:
			drawChar('n',  0x00df8090);
			break;
		case 0x18:
			drawChar('o',  0x00df8090);
			break;
		case 0x19:
			drawChar('p',  0x00df8090);
			break;
		case 0x10:
			drawChar('q',  0x00df8090);
			break;
		case 0x13:
			drawChar('r',  0x00df8090);
			break;
		case 0x1F:
			drawChar('s',  0x00df8090);
			break;
		case 0x14:
			drawChar('t',  0x00df8090);
			break;
		case 0x16:
			drawChar('u',  0x00df8090);
			break;
		case 0x2F:
			drawChar('v',  0x00df8090);
			break;
		case 0x11:
			drawChar('w',  0x00df8090);
			break;
		case 0x2D:
			drawChar('x',  0x00df8090);
			break;
		case 0x15:
			drawChar('y',  0x00df8090);
			break;
		case 0x2C:
			drawChar('z',  0x00df8090);
			break;
		case 0x02:
			drawChar('1',  0x00df8090);
			break;
		case 0x03:
			drawChar('2',  0x00df8090);
			break;
		case 0x04:
			drawChar('3',  0x00df8090);
			break;
		case 0x05:
			drawChar('4',  0x00df8090);
			break;
		case 0x06:
			drawChar('5',  0x00df8090);
			break;
		case 0x07:
			drawChar('6',  0x00df8090);
			break;
		case 0x08:
			drawChar('7',  0x00df8090);
			break;
		case 0x09:
			drawChar('8',  0x00df8090);
			break;
		case 0x0A:
			drawChar('9',  0x00df8090);
			break;
		case 0x0B:
			drawChar('0',  0x00df8090);
			break;
        // espacio en blanco
        case 0x39:
            drawChar(' ',  0x00df8090);
            break;
		case 0x1C:
			drawChar('\n',  0x00df8090);
			break;
		default:
			// drawChar(keycode);
			break;
	}
}

// Convierte KeyCodes a PinkMappings
void keycodeToPinkMap(char keycode){
	// PinkMapping (primera versión, provisoria) asigna del 0 al 9 a las teclas de 0 a 9 
	// y luego asigna las letras de la A a la Z en minúscula a las teclas de 10 a 35
	// con espacio en la 36 y enter en la 37

	// convierte el keycode a PinkMapping
	switch (keycode)
	{
		case 0x02:
			drawChar(0, 0x00df8090);
			break;
		case 0x03:
			drawChar(1, 0x00df8090);
			break;
		case 0x04:
			drawChar(2, 0x00df8090);
			break;
		case 0x05:
			drawChar(3, 0x00df8090);
			break;
		case 0x06:
			drawChar(4, 0x00df8090);
			break;
		case 0x07:
			drawChar(5, 0x00df8090);
			break;
		case 0x08:
			drawChar(6, 0x00df8090);
			break;
		case 0x09:
			drawChar(7, 0x00df8090);
			break;
		case 0x0A:
			drawChar(8, 0x00df8090);
			break;
		case 0x0B:
			drawChar(9, 0x00df8090);
			break;
		case 0x1E:
			drawChar(10, 0x00df8090);
			break;
		case 0x30:
			drawChar(11, 0x00df8090);
			break;
		case 0x2E:
			drawChar(12, 0x00df8090);
			break;
		case 0x20:
			drawChar(13, 0x00df8090);
			break;
		case 0x12:
			drawChar(14, 0x00df8090);
			break;
		case 0x21:
			drawChar(15, 0x00df8090);
			break;
		case 0x22:
			drawChar(16, 0x00df8090);
			break;
		case 0x23:
			drawChar(17, 0x00df8090);
			break;
		case 0x17:
			drawChar(18, 0x00df8090);
			break;
		case 0x24:
			drawChar(19, 0x00df8090);
			break;
		case 0x25:
			drawChar(20, 0x00df8090);
			break;
		case 0x26:
			drawChar(21, 0x00df8090);
			break;
		case 0x32:
			drawChar(22, 0x00df8090);
			break;
		case 0x31:
			drawChar(23, 0x00df8090);
			break;
		case 0x18:
			drawChar(24, 0x00df8090);
			break;
		case 0x19:
			drawChar(25, 0x00df8090);
			break;
		case 0x10:
			drawChar(26, 0x00df8090);
			break;
		case 0x13:
			drawChar(27, 0x00df8090);
			break;
		case 0x1F:
			drawChar(28, 0x00df8090);
			break;
		case 0x14:
			drawChar(29, 0x00df8090);
			break;
		case 0x16:
			drawChar(30, 0x00df8090);
			break;
		case 0x2F:
			drawChar(31, 0x00df8090);
			break;
		case 0x11:
			drawChar(32, 0x00df8090);
			break;
		case 0x2D:
			drawChar(33, 0x00df8090);
			break;
		case 0x15:
			drawChar(34, 0x00df8090);
			break;
		case 0x2C:
			drawChar(35, 0x00df8090);
			break;
		
		case 0x39:
			// espacio
			drawChar(36, 0x00df8090);
			break;
		case 0x1C:
			// enter
			drawChar(37, 0x00df8090);
			break;
		default:
			// probablemente basura
			// drawChar(keycode, 0x00df8090);
			break;
	}
}



void int_21() {
	char c = getKeyCode();
	// keycodeToAscii(c);
	keycodeToPinkMap(c);
}
