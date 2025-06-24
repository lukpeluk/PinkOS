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
#define SHIFT_KEY 0x2A // Left Shift key
#define RIGHT_SHIFT_KEY 0x36 // Right Shift key

#define SPECIAL_KEY ALT_KEY  // Acá defino qué tecla se usa para el alt-tab

extern char getKeyCode();

static int alt_pressed = 0; // Variable to track if alt is pressed
static int shift_pressed = 0; // Variable to track if shift is pressed
static int tab_pressed_with_alt = 0; // Track if tab was pressed while alt was held
static int overlay_delay_timer = 0; // Timer to delay overlay appearance


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

	// Track Alt key state
	if (event.scan_code == ALT_KEY && event.event_type == 1) {
		alt_pressed = 1;
		tab_pressed_with_alt = 0; // Reset tab flag when Alt is pressed
		return;
	}
	
	if (event.scan_code == ALT_KEY && event.event_type == 2) {
		alt_pressed = 0;
		// If tab was pressed with Alt
		if (tab_pressed_with_alt) {
			if (isWindowSwitcherActive()) {
				// If switcher is active, confirm selection
				windowSwitcherConfirm();
			} else {
				// If switcher is not active and timer still running, do direct switch
				if (overlay_delay_timer > 0) {
					overlay_delay_timer = 0; // Cancel timer
					handleDirectAltTab();
				}
			}
		}
		tab_pressed_with_alt = 0;
		overlay_delay_timer = 0; // Reset timer
		return;
	}

	// Track Shift key state
	if ((event.scan_code == SHIFT_KEY || event.scan_code == RIGHT_SHIFT_KEY) && event.event_type == 1) {
		shift_pressed = 1;
		return;
	}
	
	if ((event.scan_code == SHIFT_KEY || event.scan_code == RIGHT_SHIFT_KEY) && event.event_type == 2) {
		shift_pressed = 0;
		return;
	}

	// Handle Tab with Alt
	if (event.scan_code == TAB && alt_pressed && event.event_type == 1) {
		if (isWindowSwitcherActive()) {
			// Switcher is already active, navigate
			if (shift_pressed) {
				windowSwitcherPrev(); // Alt+Shift+Tab
			} else {
				windowSwitcherNext(); // Alt+Tab while switcher is active
			}
		} else {
			// First Alt+Tab press - mark that tab was pressed and show overlay after small delay
			tab_pressed_with_alt = 1;
			overlay_delay_timer = 5; // Small delay (will be checked in timer/main loop)
		}
		return;
	}

	// Handle escape key to cancel window switcher
	if (event.scan_code == 0x01 && event.event_type == 1 && isWindowSwitcherActive()) { // ESC key
		windowSwitcherCancel();
		tab_pressed_with_alt = 0;
		return;
	}

	// Handle arrow keys for navigation when alt is pressed and switcher is active
	if (alt_pressed && isWindowSwitcherActive()) {
		if (event.scan_code == 0x4B && event.event_type == 3) { // Left arrow
			windowSwitcherPrev();
			return;
		}
		if (event.scan_code == 0x4D && event.event_type == 3) { // Right arrow
			windowSwitcherNext();
			return;
		}
		if (event.scan_code == 0x1C && event.event_type == 1) { // Enter key
			windowSwitcherConfirm();
			tab_pressed_with_alt = 0;
			return;
		}
		// Don't process other keys when switcher is active and alt is pressed
		return;
	}

	// Handle direct window switching (Alt+Tab released immediately)
	// This is handled by the Alt release logic above

	if(event.event_type != 0) {
		handleKeyEvent(event);
	}
}

// Function to handle direct Alt+Tab switching (when Alt is released quickly)
void handleDirectAltTab() {
    if (shift_pressed) {
        switchToPrevWindow(); // Alt+Shift+Tab direct
    } else {
        switchToNextWindow(); // Alt+Tab direct
    }
}

// Function to check if overlay should be shown (call this from main loop or timer)
void checkOverlayDelay() {
    if (overlay_delay_timer > 0) {
        overlay_delay_timer--;
        
        // If timer reached 0 and Alt is still pressed, show overlay
        if (overlay_delay_timer == 0 && alt_pressed && tab_pressed_with_alt && !isWindowSwitcherActive()) {
            initWindowSwitcher();
        }
    }
}
