#include <eventHandlerManager.h>
#include <string.h>  // For memset()

// Define the kernel state structure
typedef struct KernelState {
    char pressed_keys[6];  // Array to store the currently pressed keys (up to 6) (contains scancodes, 0x00 is empty)
    KeyHandler key_handler;  // Pointer to the key press handler function
} KernelState;

static KernelState state;

// Configures a function to handle key presses, the idea is to set it via a syscall so that the shell can register events
// It can only be set once, so that a program (like snake) cannot overwrite the handler set by the main launcher
// That emulates permission levels, ideally throwing an exeption and closing the program if it tries to overwrite the handler
void setKeypressHandler(KeyHandler handler) {
    if(state.key_handler) return;  // If the key handler is already set, return

    state.key_handler = handler;
}

// Handle a key press with the configured key handler
// It's supposed to be called by the keyboard interrupt handler
// With the key PinkMapping as argument so that the user space doesn't have to deal with scancodes
void handle_key_press(char key) {
    state.key_handler(key);
}

// Function to get the current pressed keys
const char* get_pressed_keys() {
    return state.pressed_keys;
}

// Set a key as pressed, returns 0 if successful, 1 if there are no empty slots
// (normally a keyboard does the same, if you press more keys than it can handle, it will ignore the extra ones)
const char set_key(char scan_code) {
    for(int i = 0; i < 6; i++) {
        if(state.pressed_keys[i] == 0x00) {
            state.pressed_keys[i] = scan_code;
            return 0;
        }
    }

    return 1;  // If there are no empty slots, return 1
}

const char release_key(char scan_code) {
    for(int i = 0; i < 6; i++) {
        if(state.pressed_keys[i] == scan_code) {
            state.pressed_keys[i] = 0x00;
            return 0;
        }
    }

    return 1;  // If the key was not found, return 1
}

