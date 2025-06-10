#include <memoryManager/memoryManager.h>
#include <drivers/serialDriver.h>
#include <stdint.h>
#include <stddef.h>

#define HEAP_BASE_ADDRESS 0x900000  // Base del heap (diferente del stack)
#define HEAP_SIZE 0x100000000          // Tamaño total del heap (1 MB)

static uint64_t heapStart = HEAP_BASE_ADDRESS;
static uint64_t heapCurrent = HEAP_BASE_ADDRESS;
static uint64_t heapEnd = HEAP_BASE_ADDRESS + HEAP_SIZE;

// Está de onda por ahora, pero bueno, por las dudas está bueno que todo tenga su init
void initMemoryManager() {
//     log_to_serial("initMemoryManager: Iniciando el memory manager");
    heapCurrent = heapStart;
//     log_to_serial("initMemoryManager: Memory manager inicializado");
}


void* malloc(size_t size) {
    if (size == 0) {
        size = 1; // Asegurarse de que se asigna al menos 1 byte
    }

    // Alinear a 8 bytes para evitar problemas de alineación
    size_t aligned_size = (size + 7) & ~7;
    
    // Verificar si hay espacio suficiente
    if (heapCurrent + aligned_size > heapEnd) {
        // log_to_serial("malloc: Error - no hay espacio suficiente en el heap");
        return NULL;
    }

    void* allocated_ptr = (void*)heapCurrent;
    heapCurrent += aligned_size;

    // log_to_serial("malloc: Memoria asignada exitosamente");
    
    return allocated_ptr;
}

void free(void* ptr) {
    // Por ahora no hace nada
    // En el futuro se podría implementar un sistema más sofisticado
//     log_to_serial("free: Llamada a free (no implementado)");
}

void* realloc(void* ptr, size_t new_size) {
    if (ptr != NULL) {
        free(ptr);
    }

    // Simplemente asignar un nuevo chunk del tamaño pedido (sí, el realloc más de mierda que hay)
    void* new_ptr = malloc(new_size);
    if (new_ptr == NULL) {
//         log_to_serial("realloc: Error - no se pudo asignar nueva memoria");
        return NULL;
    }

//     log_to_serial("realloc: Nueva memoria asignada");
    return new_ptr;
}