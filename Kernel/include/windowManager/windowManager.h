
#include <stdint.h>

// objeto que representa la linked list de windows
// una window tiene un pid asociado, un buffer, 

typedef struct WindowControlBlock {
    uint32_t pid;                   
    uint8_t *buffer;                     // Puntero al buffer de la ventana
    struct WindowControlBlock *next;  // Siguiente ventana (lista donde el primer elemento es el foco actual, y el último es el más antiguo y apunta a NULL)
                                      // Si no hay ventanas, debería mostrar algún fondo de pantalla o algo, después lo pensamos
} WindowControlBlock;

void initWindowManager();

/** getFocusedWindow: 
 * Gets the PID of the currently focused window
 * 
 * @return The PID of the focused window, or NULL if no window is present.
*/
uint32_t getFocusedWindow();
uint32_t getWindowByPID();

int isFocusedWindow(uint32_t pid);

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
uint8_t * getBufferByPID(uint32_t pid);

// Agregar y eliminar ventanas, se encarga de allocar y liberar memoria
// Agregar una ventana devuelve un puntero al buffer de la ventana, o NULL si no se pudo allocar memoria
uint8_t * addWindow(uint32_t pid);
void removeWindow(uint32_t pid);


