#ifndef _KEYBOARD_DRIVER_H
#define _KEYBOARD_DRIVER_H

#include <stdint.h>
#include <drivers/ascii.h>


typedef struct KeyboardEvent{
    unsigned char event_type;  // 1 if pressed, 2 if released, 3 if special scancode pressed, 4 if special scancode released, 0 if no event was registered (ej. empty buffer, E0 scancode or unsupported scancode)
    int hold_times; // how many times the key was pressed before being released, 0 if it's not a press event
    unsigned char ascii;
    unsigned char scan_code;
} KeyboardEvent;

// processes the key press event, saving the key in the buffer and returning the event
KeyboardEvent processKeyPress();

int isKeyPressed(unsigned char scan_code, unsigned char is_special);    // checks if a specific key is currently being pressed, and for how many times it was held

void getKeyboardEvent(KeyboardEvent * keyboardEvent);    // dequeues from buffer
void clearKeyboardBuffer(); // clears the buffer

unsigned char keycodeToAscii(unsigned char keycode);
// unsigned char asciiToKeycode(unsigned char ascii);  // no sé si tiene sentido implementarla pero qcy

#endif

