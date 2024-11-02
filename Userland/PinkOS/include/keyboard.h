#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef struct KeyboardEvent{
    char event_type;  // 1 if pressed, 2 if released, 3 if special scancode pressed, 4 if special scancode released, 0 if no event was registered (ej. empty buffer, E0 scancode or unsupported scancode)
    int hold_times; // how many times the key was pressed before being released, 0 if it's not a press event
    char ascii;
    unsigned char scan_code;
} KeyboardEvent;

#endif