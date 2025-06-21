// utiliza la función getKeyCode que se encuentra en el archivo getKeyCode.asm
// Setea la tecla como pressed o released según el scancode, ejecuta el eventHandler correspondiente
// Además convierte el scancode a PinkMapping y lo envía a la función handle_key_press
// Habría que dejar de usar PinkMappings :( y pasar algunas cosas de la lógica al keyboardDriver

#include <stdint.h>
#include <naiveConsole.h>
#include <eventHandling/eventHandlers.h>
#include <eventManager/eventManager.h>
#include <drivers/videoDriver.h>
#include <drivers/keyboardDriver.h>
#include <drivers/registersDriver.h>

#include <windowManager/windowManager.h>

#define BACKUP_REGISTERS_KEY 0x3B // F1
#define TAB 0x0F // Tab key
#define CONTROL_KEY 0x1D // Control key
#define ALT_KEY 0x38 // Alt key

#define SPECIAL_KEY ALT_KEY  // Acá defino qué tecla se usa para el alt-tab

extern char getKeyCode();

static int index = 0;
static special_key_pressed = 0; // Variable to track if ctrl is pressed


void simulateKeyCode(unsigned char scan_code) {
	KeyboardEvent e = processScancode(scan_code);

	if(e.event_type != 0) {
		handleKeyEvent(e);
	}
}


// assumes scan code set is 1
void int_21() {
	KeyboardEvent event = processKeyPress();
	// if(event.scan_code == BACKUP_REGISTERS_KEY && event.event_type == 1) {
	// 	callRegistersHandler(getBackupRegisters());
	// }
	// callKeyHandler(event.event_type, event.hold_times, event.ascii, event.scan_code);


	// Este código es para manejar el alt tab para cambiar de ventana, no debería estar en kernel pero bueno, para probarlo sirve
	if (event.scan_code == TAB && special_key_pressed && event.event_type == 1) {
		// callRegistersHandler(getBackupRegisters());
		index++;
	}	

	if(event.scan_code == SPECIAL_KEY && event.event_type == 1) {
		special_key_pressed = 1; // Set ctrl pressed state on key press
		index = 0; // Reset index when ctrl is pressed
	}

	if(event.scan_code == SPECIAL_KEY && event.event_type == 2) {
		special_key_pressed = 0; // Reset ctrl pressed state on key release

		Pid * windows = getWindows();
		int length = 0;
		while (windows[length] != 0) {
			length++;
		}
		if(index >= length) {
			index = length-1;
		}

		switchToWindow(windows[index]); // Switch to the other window (assuming PIDs 1 and 2 are the two windows)
		index = 0; // Reset index on key release

		simulateKeyCode(0x3F);  // F5 press
		simulateKeyCode(0xBF);  // F5 release
	}



	if(event.event_type != 0) {
		handleKeyEvent(event);
	}
}
