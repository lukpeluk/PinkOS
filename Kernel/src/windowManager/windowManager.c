
#include <windowManager/windowManager.h>
#include <drivers/videoDriver.h>
#include <drivers/serialDriver.h>
#include <memoryManager/memoryManager.h>
#include <types.h>


typedef struct WindowControlBlock {
    uint32_t pid;
    int redraw;
    uint8_t *buffer;                     // Puntero al buffer de la ventana
    struct WindowControlBlock *next;  // Siguiente ventana (lista donde el primer elemento es el foco actual, y el último es el más antiguo y apunta a NULL)
                                      // Si no hay ventanas, debería mostrar algún fondo de pantalla o algo, después lo pensamos
} WindowControlBlock;

static WindowControlBlock *focusedWindow = NULL;  // Proceso actualmente en ejecución
static uint8_t *overlayBuffer;   
static int overlay_enabled = 0; // Si está habilitado el overlay, esto es por optimización, si todo el tiempo copiábamos el overlay el video driver iba medio lento

void initWindowManager(){
    overlayBuffer = createVideoBuffer();
    // drawRectangle(overlayBuffer, (Point *) {50, 50}, (Point *) {90, 90}, 0x000000); 
};

Pid getFocusedWindow(){
    if (focusedWindow == NULL) {
        return 0;
    }
    return focusedWindow->pid;
}

int isFocusedWindow(Pid pid){
    if (focusedWindow == NULL) {
        return 0; 
    }
    return focusedWindow->pid == pid;
}

uint8_t * getFocusedBuffer(){
    if (focusedWindow == NULL || !focusedWindow->redraw) {
        return NULL; 
    }
    return focusedWindow->buffer;
}

// Null no significa error, significa que el overlay no está habilitado
uint8_t * getOverlayBuffer(){
    if(!overlay_enabled){
        return NULL;
    }
    return overlayBuffer;
}

void toggleOverlay(){
    overlay_enabled = !overlay_enabled;
}

WindowControlBlock * getWindowBlock(Pid pid){
    WindowControlBlock *currentWindow = focusedWindow;
    Process parent;

    Process currentProcess = getProcess(pid);
    if (currentProcess.pid == 0) {
        return NULL; // Proceso no encontrado
    }

    parent = getParent(pid);
    if (parent.pid == 0) {
        log_to_serial("E: getBufferByPID: Proceso no tiene padre");
        log_decimal("E: getBufferByPID: PID: ", pid);
    }
    while (currentWindow != NULL) {
        if (currentWindow->pid == pid) {
            return currentWindow;
        } else if (currentProcess.type == PROCESS_TYPE_THREAD && parent.pid != 0 && parent.pid == currentWindow->pid) {  // Caso especial para que el thread use el buffer de su padre
            return currentWindow;
        }
        currentWindow = currentWindow->next;
    }
    return NULL; // No se encontró la ventana con el PID especificado
}


uint8_t * getBufferByPID(Pid pid){
    WindowControlBlock *currentWindow = getWindowBlock(pid);
    if (currentWindow == NULL) {
        return NULL; // No se encontró la ventana con el PID especificado
    }
    return currentWindow->buffer;
}


// TODO: capaz permitir elegir si focusear la ventana o si ponerla como segunda
int addWindow(Pid pid){
    WindowControlBlock *newWindow = (WindowControlBlock *)malloc(sizeof(WindowControlBlock));
    if (newWindow == NULL) {
        // log_to_serial("addWindow: Error al allocar memoria para la nueva ventana");
        return -1; // No se pudo allocar memoria
    }
    newWindow->pid = pid;
    newWindow->redraw = 1; // Por defecto, la ventana necesita ser redibujada
    newWindow->buffer = createVideoBuffer(); // el video driver se encarga de allocar memoria para el buffer con la resolución adecuada

    if (newWindow->buffer == NULL) {
        // log_to_serial("addWindow: Error al crear el buffer de video");
        free(newWindow); // Liberar memoria si no se pudo crear el buffer
        return -1; // No se pudo allocar memoria para el buffer
    }

    // Insertar al principio de la lista, ahora se volvió el foco actual
    newWindow->next = focusedWindow; 
    focusedWindow = newWindow;

    return 0; // Éxtio
}

int removeWindow(Pid pid){
    if (focusedWindow == NULL) {
        // log_to_serial("removeWindow: No hay ventanas para eliminar");
        return -1; // No hay ventanas para eliminar
    }

    WindowControlBlock *current = focusedWindow;
    WindowControlBlock *prev = NULL;
    while (current != NULL) {
        if (current->pid == pid) {
            // Si es el primer elemento, actualizar el foco
            if (prev == NULL) {
                focusedWindow = current->next;
            } else {
                prev->next = current->next; // Eliminar de la lista
            }
            free(current->buffer); // Liberar el buffer de video
            free(current); // Liberar la memoria del WindowControlBlock
            return 0; // Eliminación exitosa
        }
        prev = current;
        current = current->next;
    }
    // log_to_serial("removeWindow: No se encontro la ventana con el PID especificado");
    return -1; // No se encontró la ventana con el PID especificado
}

// Aguanten las linked lists
// Alt tab (le indicás el pid al cual cambiar)
int switchToWindow(Pid pid) {
    if (focusedWindow == NULL) {
        // log_to_serial("switchWindow: No hay ventanas para cambiar");
        return -1; // No hay ventanas para cambiar
    }

    WindowControlBlock *current = focusedWindow;
    WindowControlBlock *prev = NULL;
    while (current != NULL) {
        if (current->pid == pid) {
            // Si la ventana ya es el foco, no hacer nada
            if (current == focusedWindow) {
                return 0; // Ya es el foco actual
            }
            
            // Remover la ventana de su posición actual
            prev->next = current->next;
            
            // Mover la ventana al frente de la lista
            current->next = focusedWindow;
            focusedWindow = current;
            
            return 1; // Cambio exitoso
        }
        prev = current;
        current = current->next;
    }

    // log_to_serial("switchWindow: No se encontro la ventana con el PID especificado");
    return -1; // No se encontró la ventana con el PID especificado
}

// Devuelve los pids de las ventanas abiertas, iterando por esto y buscando el nombre del programa asociado al proceso implementás alt+tab
// Null terminated
// Es tarea de quien llame a esta función liberar la memoria del arreglo devuelto
Pid * getWindows(){
    int count = 0;
    WindowControlBlock *current = focusedWindow;

    // Contar la cantidad de ventanas
    while (current != NULL) {
        count++;
        current = current->next;
    }

    if (count == 0) {
        return NULL;
    }

    Pid *pids = (Pid *)malloc(sizeof(Pid) * count + 1); // +1 para el NULL al final
    if (pids == NULL) {
        // log_to_serial("getWindows: Error al allocar memoria para el arreglo de PIDs");
        return NULL;
    }

    current = focusedWindow;
    for (int i = 0; i < count; i++) {
        pids[i] = current->pid;
        current = current->next;
    }

    pids[count] = 0; // NULL al final para indicar el final de la lista

    return pids;
}

void setRedrawFlag(Pid pid, int redraw){
    WindowControlBlock *windowBlock = getWindowBlock(pid);
    if (windowBlock == NULL) {
        log_to_serial("E: setRedrawFlag: No se encontro la ventana con el PID especificado");
        return; // No se encontró la ventana con el PID especificado
    }
    windowBlock->redraw = redraw; // Actualizar el flag de redibujado
}