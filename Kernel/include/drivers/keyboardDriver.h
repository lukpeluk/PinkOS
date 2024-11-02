#ifndef _KEYBOARD_DRIVER_H
#define _KEYBOARD_DRIVER_H

#include <stdint.h>
#include <drivers/ascii.h>


typedef struct KeyboardEvent{
    char event_type;  // 1 if pressed, 2 if released, 3 if special scancode pressed, 4 if special scancode released, 0 if no event was registered or it's unsupported (ej. empty buffer or unsupported scancode)
    char ascii;
    char scan_code;
} KeyboardEvent;

// processes the key press event, saving the key in the buffer and returning the event
KeyboardEvent processKeyPress();

int isKeyPressed(char scan_code, char is_special);    // checks if a specific key is currently being pressed)
KeyboardEvent getKeyboardEvent();    // dequeues from buffer
void clearKeyboardBuffer(); // clears the buffer

char keycodeToAscii(char keycode);
// char asciiToKeycode(char ascii);  // no sé si tiene sentido implementarla pero qcy

#endif

