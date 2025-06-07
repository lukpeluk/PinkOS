
#include <stdint.h>

// objeto que representa la linked list de windows
// una window tiene un pid asociado, un buffer, 


/* * getFocusedWindow: 
 * Gets the PID of the currently focused window
 * 
 * @return The PID of the focused window, or NULL if no window is present.
*/
uint32_t getFocusedWindow();

/* * getFocusedBuffer: 
 * Gets the buffer of the currently focused window
 * 
 * @return A pointer to the buffer of the focused window, or NULL if no window is present.
*/
void * getFocusedBuffer();




