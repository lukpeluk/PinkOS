
#include <windowManager/windowManager.h>
#include <drivers/videoDriver.h>

#define NULL 0

static WindowControlBlock *focusedWindow = NULL;  // Proceso actualmente en ejecución

void initWindowManager();

// ojo piojo, no puedo tener un PID 0
uint32_t getFocusedWindow(){
    if (focusedWindow == NULL) {
        return NULL;
    }
    return focusedWindow->pid;
}

int isFocusedWindow(uint32_t pid){
    if (focusedWindow == NULL) {
        return 0; 
    }
    return focusedWindow->pid == pid;
}

uint8_t * getFocusedBuffer(){
    if (focusedWindow == NULL) {
        return NULL; 
    }
    return focusedWindow->buffer;
}

// Agregar y eliminar ventanas, se encarga de allocar y liberar memoria
// Agregar una ventana devuelve un puntero al buffer de la ventana, o NULL si no se pudo allocar memoria
// TODO: capaz permitir elegir si focusear la ventana o si ponerla como segunda
uint8_t * addWindow(uint32_t pid){
    WindowControlBlock *newWindow = (WindowControlBlock *)malloc(sizeof(WindowControlBlock));
    if (newWindow == NULL) {
        log_to_serial("addWindow: Error al allocar memoria para la nueva ventana");
        return NULL; // No se pudo allocar memoria
    }
    newWindow->pid = pid;
    newWindow->buffer = createVideoBuffer(); // el video driver se encarga de allocar memoria para el buffer con la resolución adecuada

    if (newWindow->buffer == NULL) {
        log_to_serial("addWindow: Error al crear el buffer de video");
        free(newWindow); // Liberar memoria si no se pudo crear el buffer
        return NULL; // No se pudo allocar memoria para el buffer
    }

    // Insertar al principio de la lista, ahora se volvió el foco actual
    newWindow->next = focusedWindow; 
    focusedWindow = newWindow;
}


void removeWindow(uint32_t pid);


