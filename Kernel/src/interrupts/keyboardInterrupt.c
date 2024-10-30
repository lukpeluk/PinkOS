// utiliza la función getKeyCode que se encuentra en el archivo getKeyCode.asm
// Setea la tecla como pressed o released según el scancode, ejecuta el eventHandler correspondiente
// Además convierte el scancode a PinkMapping y lo envía a la función handle_key_press
// Habría que dejar de usar PinkMappings :( y pasar algunas cosas de la lógica al keyboardDriver

#include <stdint.h>
#include <naiveConsole.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <drivers/keyboardDriver.h>

extern char getKeyCode();

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

		int ascii = keycodeToAscii(c);
		if(ascii != 0 && eventHandlers.key_handler != 0)
			eventHandlers.key_handler(ascii);
	}
	else if (c < 0xD9) {
		release_key(c - 0x80);
	} else {
		// unsupported scancode
	}

		
}
