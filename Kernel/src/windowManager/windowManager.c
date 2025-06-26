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
static uint8_t *overlay_buffer;   
static int overlay_enabled = 0; // Si está habilitado el overlay, esto es por optimización, si todo el tiempo copiábamos el overlay el video driver iba medio lento

// ===================== WINDOW SWITCHER FUNCTIONALITY =====================

// Static variables for window switcher state
static int window_switcher_active = 0;
static int selected_window_index = 0;
static Pid *cached_windows = NULL;
static int cached_window_count = 0;

void initWindowManager(){
    overlay_buffer = createVideoBuffer();
    // drawRectangle(overlay_buffer, (Point *) {50, 50}, (Point *) {90, 90}, 0x000000); 
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
    return overlay_buffer;
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

void initWindowSwitcher() {
    // Don't activate if no windows available
    cached_windows = getWindows();
    if (cached_windows == NULL) {
        cached_window_count = 0;
        return;
    }
    
    // Count windows
    cached_window_count = 0;
    while (cached_windows[cached_window_count] != 0) {
        cached_window_count++;
    }
    
    if (cached_window_count == 0) {
        // Not enough windows to switch, free memory and return
        free(cached_windows);
        cached_windows = NULL;
        cached_window_count = 0;
        return;
    }
    
    // Activate switcher
    window_switcher_active = 1;
    selected_window_index = 0;
    
    // Find currently focused window and set it as selected
    Pid focused = getFocusedWindow();
    for (int i = 0; i < cached_window_count; i++) {
        if (cached_windows[i] == focused) {
            selected_window_index = i;
            break;
        }
    }
    
    // Enable overlay to show switcher
    if (!overlay_enabled) {
        toggleOverlay();
    }
    
    console_log("Window switcher activated with %d windows", cached_window_count);
}

void windowSwitcherNext() {
    if (!window_switcher_active || cached_window_count == 0) {
        return;
    }
    
    selected_window_index = (selected_window_index + 1) % cached_window_count;
    console_log("Window switcher: next -> index %d", selected_window_index);
}

void windowSwitcherPrev() {
    if (!window_switcher_active || cached_window_count == 0) {
        return;
    }
    
    selected_window_index = (selected_window_index - 1 + cached_window_count) % cached_window_count;
    console_log("Window switcher: prev -> index %d", selected_window_index);
}

void windowSwitcherConfirm() {
    if (!window_switcher_active || cached_window_count == 0) {
        windowSwitcherCancel();
        return;
    }
    
    // Switch to selected window
    Pid selected_pid = cached_windows[selected_window_index];
    switchToWindow(selected_pid);
    
    console_log("Window switcher: confirmed switch to PID %d", selected_pid);
    
    // Clean up and deactivate
    window_switcher_active = 0;
    if (cached_windows != NULL) {
        free(cached_windows);
        cached_windows = NULL;
    }
    cached_window_count = 0;
    selected_window_index = 0;
    
    // Disable overlay
    if (overlay_enabled) {
        toggleOverlay();
    }
}

void windowSwitcherCancel() {
    if (!window_switcher_active) {
        return;
    }
    
    log_to_serial("Window switcher: cancelled");
    
    // Clean up and deactivate without switching
    window_switcher_active = 0;
    if (cached_windows != NULL) {
        free(cached_windows);
        cached_windows = NULL;
    }
    cached_window_count = 0;
    selected_window_index = 0;
    
    // Disable overlay
    if (overlay_enabled) {
        toggleOverlay();
    }
}

int isWindowSwitcherActive() {
    return window_switcher_active;
}

int getSelectedWindowIndex() {
    if (!window_switcher_active) {
        return -1;
    }
    return selected_window_index;
}

void windowManagerDrawOverlay() {
    // Only draw if window switcher is active
    if (!window_switcher_active) {
        return;
    }
    
    // Clear the overlay buffer
    memset(overlay_buffer, 0, getScreenWidth() * getScreenHeight() * 3); // Assuming 24-bit color
    
    uint64_t screen_width = getScreenWidth();
    uint64_t screen_height = getScreenHeight();
    
    if (cached_windows == NULL || cached_window_count == 0) {
        drawStringAt(overlay_buffer, "No windows available", 0xFFFFFF, 0x000000, 
                    &(Point){screen_width / 2 - 80, screen_height / 2});
        return;
    }
    
    // Calculate layout for window previews
    uint64_t preview_width = 200;
    uint64_t preview_height = 150;
    uint64_t spacing = 20;
    uint64_t total_width = (preview_width * cached_window_count) + (spacing * (cached_window_count - 1));
    
    // Center the layout on screen
    uint64_t start_x = (screen_width - total_width) / 2;
    uint64_t start_y = (screen_height - preview_height) / 2;
    
    // Draw background panel
    uint64_t panel_padding = 30;
    drawRectangle(overlay_buffer, 
                 &(Point){start_x - panel_padding, start_y - panel_padding}, 
                 &(Point){start_x + total_width + panel_padding, start_y + preview_height + panel_padding + 60}, 
                 0x2A2A2A);
    drawRectangleBoder(overlay_buffer, 
                      &(Point){start_x - panel_padding, start_y - panel_padding}, 
                      &(Point){start_x + total_width + panel_padding, start_y + preview_height + panel_padding + 60}, 
                      2, 0x555555);
    
    // Draw each window preview
    for (int i = 0; i < cached_window_count; i++) {
        Pid current_pid = cached_windows[i];
        uint8_t * window_buffer = getBufferByPID(current_pid);
        
        uint64_t preview_x = start_x + i * (preview_width + spacing);
        uint64_t preview_y = start_y;
        
        // Highlight selected window
        int is_selected = (i == selected_window_index);
        uint32_t border_color = is_selected ? 0x00AAFF : 0x666666;
        uint32_t border_thickness = is_selected ? 4 : 1;
        uint32_t bg_color = is_selected ? 0x2A2A3A : 0x1A1A1A;
        
        // Draw preview background
        drawRectangle(overlay_buffer, 
                     &(Point){preview_x, preview_y}, 
                     &(Point){preview_x + preview_width, preview_y + preview_height}, 
                     bg_color);
        
        // Draw simplified window content if buffer is available
        if (window_buffer != NULL) {
            uint64_t scale_x = screen_width / preview_width;
            uint64_t scale_y = screen_height / preview_height;
            
            for (uint64_t py = 0; py < preview_height; py++) {
                for (uint64_t px = 0; px < preview_width; px++) {
                    uint64_t src_x = px * scale_x;
                    uint64_t src_y = py * scale_y;
                    
                    if (src_x < screen_width && src_y < screen_height) {
                        uint64_t src_offset = (src_x * 3) + (src_y * screen_width * 3); // Assuming 24-bit color
                        uint32_t pixel = *(uint32_t *)(window_buffer + src_offset) & 0x00FFFFFF;
                        if (pixel == 0x000000) pixel = 0x010101; // Replace transparent pixels with a light gray for visibility
                        putPixel(overlay_buffer, pixel, preview_x + px, preview_y + py);
                    }
                }
            }
        } else {
            drawStringAt(overlay_buffer, "No preview", 0xFFFFFF, 0x000000, 
                        &(Point){preview_x + 50, preview_y + 70});
        }
        
        // Draw border around preview
        drawRectangleBoder(overlay_buffer, 
                          &(Point){preview_x, preview_y}, 
                          &(Point){preview_x + preview_width, preview_y + preview_height}, 
                          border_thickness, border_color);
        
        // Draw PID label below preview
        char pid_label[20];
        int pid_int = (int)current_pid;
        int label_len = 0;
        if (pid_int == 0) {
            pid_label[0] = '0';
            label_len = 1;
        } else {
            int temp = pid_int;
            while (temp > 0) {
                temp /= 10;
                label_len++;
            }
            for (int j = label_len - 1; j >= 0; j--) {
                pid_label[j] = (pid_int % 10) + '0';
                pid_int /= 10;
            }
        }
        pid_label[label_len] = '\0';
        
        uint32_t label_color = is_selected ? 0xFFFFFF : 0xCCCCCC;
        uint32_t muted_label_color = is_selected ? 0xAAAAAA : 0x888888;
        drawStringAt(overlay_buffer, "PID: ", muted_label_color, 0x2A2A2A, 
                    &(Point){preview_x + 10, preview_y + preview_height + 10});
        drawStringAt(overlay_buffer, pid_label, muted_label_color, 0x2A2A2A, 
                    &(Point){preview_x + 60, preview_y + preview_height + 10});

        // Draw program name below PID
        Process process = getProcess(current_pid);
        char * program_name = process.program.name;
        if (program_name == NULL || strlen(program_name) == 0) {
            program_name = "Unknown";
        }
        // uint64_t name_length = strlen(program_name) * getCharWidth();

        // Start position same as PID label,left aligned
        Point name_position = {preview_x + 10, preview_y + preview_height + 30};
        drawStringAt(overlay_buffer, program_name, label_color, 0x2A2A2A, 
                    &name_position);
    }
    // Center in x axis
    char * bottom_text = "Alt+Tab: Navigate  Alt+Shift+Tab: Back  Esc: Cancel";
    uint64_t text_length = strlen(bottom_text) * getCharWidth();
    Point text_position = {screen_width / 2 - text_length / 2, screen_height - 64};
    Point rect_start = {text_position.x - 10, text_position.y - 10};
    Point rect_end = {text_position.x + text_length + 10, text_position.y + 10 + getCharHeight()};
    // Draw instructions centered at bottom of screen
    drawRectangle(overlay_buffer, &rect_start, &rect_end, 0x2A2A2A);
    drawStringAt(overlay_buffer, bottom_text, 0xAAAAAA, 0x2A2A2A, 
                &text_position);
                
}

// Function to switch to next window without showing overlay (direct Alt+Tab)
void switchToNextWindow() {
    Pid * windows = getWindows();
    if (windows == NULL) {
        return;
    }
    
    // Count windows
    int window_count = 0;
    while (windows[window_count] != 0) {
        window_count++;
    }
    
    if (window_count <= 1) {
        free(windows);
        return; // Not enough windows to switch
    }
    
    // Find currently focused window
    Pid focused = getFocusedWindow();
    int current_index = 0;
    for (int i = 0; i < window_count; i++) {
        if (windows[i] == focused) {
            current_index = i;
            break;
        }
    }
    
    // Switch to next window
    int next_index = (current_index + 1) % window_count;
    switchToWindow(windows[next_index]);
    
    console_log("Direct switch to next window PID %d", windows[next_index]);
    free(windows);
}

// Function to switch to previous window without showing overlay (direct Alt+Shift+Tab)
void switchToPrevWindow() {
    Pid * windows = getWindows();
    if (windows == NULL) {
        return;
    }
    
    // Count windows
    int window_count = 0;
    while (windows[window_count] != 0) {
        window_count++;
    }
    
    if (window_count <= 1) {
        free(windows);
        return; // Not enough windows to switch
    }
    
    // Find currently focused window
    Pid focused = getFocusedWindow();
    int current_index = 0;
    for (int i = 0; i < window_count; i++) {
        if (windows[i] == focused) {
            current_index = i;
            break;
        }
    }
    
    // Switch to previous window
    int prev_index = (current_index - 1 + window_count) % window_count;
    switchToWindow(windows[prev_index]);
    
    console_log("Direct switch to prev window PID %d", windows[prev_index]);
    free(windows);
}