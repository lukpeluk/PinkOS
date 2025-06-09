#include <stdint.h>
#include <stdlib.h>
#include <fileSystem/fileSystem.h>

// A futuro podría guardar una lista de quién lo abrió para permitir cosas como no permitir escritura mientras alguien lo está leyendo
// Por ahora igual los procesos pueden usar Mutexes para sincronizarse entre ellos así que no es prioritario
typedef struct {
    File file;        // Información del archivo
    uint8_t *data;    // Puntero a los datos del archivo
    uint64_t readPointer;  // Puntero de lectura FIFO
    uint64_t writePointer; // Puntero de escritura FIFO
    FifoFileControlBlock *next; // Linked list de archivos (para mejor eficiencia podría ser un árbol pero bueno, por ahora es suficiente)
} FifoFileControlBlock;

typedef struct {
    File file;        // Información del archivo
    uint8_t *data;    // Puntero a los datos del archivo
    FifoFileControlBlock *next; // Linked list de archivos (para mejor eficiencia podría ser un árbol pero bueno, por ahora es suficiente)
} FifoFileControlBlock;
