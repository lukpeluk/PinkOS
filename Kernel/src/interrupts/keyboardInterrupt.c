// utiliza la función getKeyCode que se encuentra en el archivo getKeyCode.asm
// Setea la tecla como pressed o released según el scancode, ejecuta el eventHandler correspondiente
// Además convierte el scancode a PinkMapping y lo envía a la función handle_key_press
// Habría que dejar de usar PinkMappings :( y pasar algunas cosas de la lógica al keyboardDriver

#include <stdint.h>
#include <naiveConsole.h>
#include <eventHandling/eventHandlers.h>
#include <drivers/videoDriver.h>
#include <drivers/keyboardDriver.h>
#include <drivers/registersDriver.h>

#define BACKUP_REGISTERS_KEY 0x3B // F1

extern unsigned char getKeyCode();

// assumes scan code set is 1
void int_21() {
	KeyboardEvent event = processKeyPress();
	if(event.scan_code == BACKUP_REGISTERS_KEY && event.event_type == 1) {
		callRegistersHandler(getBackupRegisters());
	}
	callKeyHandler(event.event_type, event.hold_times, event.ascii, event.scan_code);	
}
