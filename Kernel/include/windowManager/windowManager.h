
#include <stdint.h>
#include <processManager/scheduler.h>

// objeto que representa la linked list de windows
// una window tiene un pid asociado, un buffer, 

void initWindowManager();

/** getFocusedWindow: 
 * Gets the PID of the currently focused window
 * 
 * @return The PID of the focused window, or NULL if no window is present.
*/
Pid getFocusedWindow();
int isFocusedWindow(Pid pid);

/** getFocusedBuffer: 
 * Gets the buffer of the currently focused window
 * 
 * @return A pointer to the buffer of the focused window, or NULL if no window is present.
*/
uint8_t * getFocusedBuffer();

/** getBufferByPID: 
 * Gets the buffer of a window by its PID
 * 
 * @param pid The PID of the window to get the buffer for.
 * @return A pointer to the buffer of the window with the given PID, or NULL if no such window exists.
*/
uint8_t * getBufferByPID(Pid pid);

// Agregar y eliminar ventanas, se encarga de allocar y liberar memoria
// Agregar una ventana devuelve un puntero al buffer de la ventana, o NULL si no se pudo allocar memoria
int addWindow(Pid pid);
int removeWindow(Pid pid);

/** switchToWindow: 
 * Switches focus to a window with the given PID
 * 
 * @param pid The PID of the window to switch to.
 * @return 1 if the switch was successful, 0 if the window is already focused, or -1 if no such window exists.
*/
int switchToWindow(Pid pid);

// Devuelve los pids de las ventanas abiertas, null terminated, iterando por esto y buscando el nombre del programa asociado al proceso implementás alt+tab
Pid * getWindows();

void setRedrawFlag(Pid pid, int redraw);


