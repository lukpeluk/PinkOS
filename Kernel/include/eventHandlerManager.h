#ifndef EVENT_HANDLER_MANAGER_H
#define EVENT_HANDLER_MANAGER_H

// Define a type for the key press handler function pointer
typedef void (*KeyHandler)(char key);

// Function to initialize the kernel state
void setKeypressHandler(KeyHandler handler);

// Handle a key press with the configured key handler
// It's supposed to be called by the keyboard interrupt handler
// With the key PinkMapping as argument so that the user space doesn't have to deal with scancodes
void handle_key_press(char key);

// Function to get the current pressed keys (scan codes)
const char* get_pressed_keys();

// Set and release keys
// Intended to be used by the keyboard interrupt handler
const char set_key(char scan_code);
const char release_key(char scan_code);

#endif
