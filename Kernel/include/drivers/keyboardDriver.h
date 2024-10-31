#ifndef _KEYBOARD_DRIVER_H
#define _KEYBOARD_DRIVER_H

#include <stdint.h>
#include <drivers/ascii.h>


typedef struct KeyboardEvent{
    char event_type; // 1 if pressed, 2 if released, 3 if other (not yet supported), 0 if no event was registered (empty buffer)
    char ascii;
    char scan_code;
} KeyboardEvent;

// processes the key press event, saving the key in the buffer and returning the event
KeyboardEvent processKeyPress();

int isKeyPressed(char scan_code);    // checks if a key is currently being pressed
KeyboardEvent getKeyboardEvent();    // dequeues from buffer

char keycodeToAscii(char keycode);
// char asciiToKeycode(char ascii);  // no sé si tiene sentido implementarla pero qcy

#endif

