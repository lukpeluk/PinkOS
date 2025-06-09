
#include <windowManager/windowManager.h>
#include <drivers/videoDriver.h>

#define NULL 0

static WindowControlBlock *focusedWindow = NULL;  // Proceso actualmente en ejecución

void initWindowManager(){};

Pid getFocusedWindow(){
    if (focusedWindow == NULL) {
        return NULL;
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
    if (focusedWindow == NULL) {
        return NULL; 
    }
    return focusedWindow->buffer;
}

uint8_t * getBufferByPID(Pid pid){
    WindowControlBlock *current = focusedWindow;
    while (current != NULL) {
        if (current->pid == pid) {
            return current->buffer;
        }
        current = current->next;
    }
    return NULL; // No se encontró la ventana con el PID especificado
}

// TODO: capaz permitir elegir si focusear la ventana o si ponerla como segunda
uint8_t * addWindow(Pid pid){
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

int removeWindow(Pid pid){
    if (focusedWindow == NULL) {
        log_to_serial("removeWindow: No hay ventanas para eliminar");
        return; 
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
    log_to_serial("removeWindow: No se encontró la ventana con el PID especificado");
    return -1; // No se encontró la ventana con el PID especificado
}

// Aguanten las linked lists
// Alt tab (le indicás el pid al cual cambiar)
int switchToWindow(Pid pid) {
    if (focusedWindow == NULL) {
        log_to_serial("switchWindow: No hay ventanas para cambiar");
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

    log_to_serial("switchWindow: No se encontró la ventana con el PID especificado");
    return -1; // No se encontró la ventana con el PID especificado
}


// Devuelve los pids de las ventanas abiertas, iterando por esto y buscando el nombre del programa asociado al proceso implementás alt+tab
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

    Pid *pids = (Pid *)malloc(sizeof(Pid) * count);
    if (pids == NULL) {
        log_to_serial("getWindows: Error al allocar memoria para el arreglo de PIDs");
        return NULL;
    }

    current = focusedWindow;
    for (int i = 0; i < count; i++) {
        pids[i] = current->pid;
        current = current->next;
    }

    return pids;
}