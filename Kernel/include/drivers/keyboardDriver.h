#ifndef _KEYBOARD_DRIVER_H
#define _KEYBOARD_DRIVER_H

#include <stdint.h>

// Function to get the current pressed keys
// It returns a pointer to the array of pressed keys, so if the user wants to modify it, technically it can,
// But it would be ignoring the const qualifier and shouldn't be done
// The function could recieve a pointer to an array as an argument and return there, but for simplicity I think this is fine (to-do?)
const char* get_pressed_keys();

// TODO: esto no debería estar expuesto, debería ser privado y el handleKeyPress usarlo internamente
// pasa que ahora la lógica está en la interrupción
const char set_key(char scan_code);
const char release_key(char scan_code);


char keycodeToAscii(char keycode);
int keycodeToPinkMap(char keycode); //! (DEPRECATED)

#endif

