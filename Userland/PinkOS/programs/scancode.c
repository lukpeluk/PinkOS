#include <keyboard.h>
#include <stdpink.h>
#include <syscalls/syscall.h>


void scancode_printer_main(char *args) {
    syscall(CLEAR_KEYBOARD_BUFFER_SYSCALL, 0, 0, 0, 0, 0);

    print((char *)"Press any key to see its scancode, press ESC to exit.\nIf you press a key and nothing happens, the key is probably not supported.\n\n");

    while (1) {
        KeyboardEvent event = getKeyboardEvent();
        
        if (event.event_type == 0) {
            continue; // No event (or unsupported)
        }
        
        switch (event.event_type) {
        case 1:
            printf("Key pressed:\n    Scancode: 0x%X (%d)\n    Ascii: %d (%c)\n    Hold times: %d\n\n", event.scan_code, event.scan_code, event.ascii, event.ascii, event.hold_times);
            break;
        case 2:
            printf("Key released:\n    Scancode: 0x%X (%d)\n    Ascii: %d (%c)\n    Hold times: %d\n\n", event.scan_code, event.scan_code, event.ascii, event.ascii, event.hold_times);
            break;
        case 3:
            printf("Key pressed:\n    Scancode (special): 0x%X (%d)\n    Ascii: %d (%c)\n    Hold times: %d\n\n", event.scan_code, event.scan_code, event.ascii, event.ascii, event.hold_times);
            break;
        case 4:
            printf("Key released:\n    Scancode (special): 0x%X (%d)\n    Ascii: %d (%c)\n    Hold times: %d\n\n", event.scan_code, event.scan_code, event.ascii, event.ascii, event.hold_times);
            break;
        
        default:
            break;
        }
    }
}