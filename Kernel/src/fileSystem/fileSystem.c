#include <stdint.h>
#include <stdlib.h>
#include <fileSystem/fileSystem.h>
#include <memoryManager/memoryManager.h>
#include <processManager/scheduler.h>
#include <drivers/serialDriver.h>   
#include <lib.h>

#define ADVANCE_FIFO_POINTER(pointer, size) \
    pointer = (pointer + 1) % size;

#define FIFO_FULL(fileBlock) \
    (fileBlock->writtenBytes == fileBlock->file.size)
    
#define FIFO_EMPTY(fileBlock) \
    (fileBlock->writtenBytes == 0)


typedef struct InternalFilePermissions{
    Pid writing_owner;
    Program writing_owner_program; // El programa del owner, para poder validar si es del mismo programa que el proceso que quiere acceder
    char writing_conditions;

    Pid reading_owner;
    Program reading_owner_program;
    char reading_conditions;
} InternalFilePermissions;


/*  
    * A futuro se podría guardar una lista de quién abrió cada archivo para cosas como opcionalmente no permitir escritura mientras alguien tiene abierto el archivo
      -> Por ahora igual los procesos pueden usar Mutexes para sincronizarse entre ellos así que no es prioritario

    * Los archivos FIFO tienen un buffer circular, donde que no haya espacio significa que el puntero de escritura alcanzó al de lectura, y que se leyó todo la inversa
    * Los file control blocks tienen información extra necesaria para operar con el archivo, como permisos, el puntero a la data, etc...
    * Para recorrer los archivos, hay una linked list de los archivos de cada tipo, donde los nodos son los file control blocks
        -> Para mejor eficiencia de búsqueda podría ser un árbol pero bueno, por ahora es suficiente
*/

typedef struct FifoFileControlBlock {
    File file;        // Información del archivo
    uint8_t *data;    // Puntero a los datos del archivo
    InternalFilePermissions permissions;
    uint64_t readPointer;   // Puntero de lectura
    uint64_t writePointer;  // Puntero de escritura
    uint32_t writtenBytes;  // Cantidad de bytes escritos (para distinguir buffer lleno de buffer vacío, problemita que teníamos...)
    int closed_for_writing; // Indica si el archivo se cerró para escritura (nunca más va a haber datos nuevos, así que llegar al final da un EOF y borra el archivo)
    struct FifoFileControlBlock *next;
} FifoFileControlBlock;

typedef struct RawFileControlBlock {
    File file;       // Información general (pública) del archivo
    uint8_t *data;   // Puntero a los datos del archivo
    InternalFilePermissions permissions;
    struct RawFileControlBlock *next;
} RawFileControlBlock;


// Variables globales del filesystem
// Se guardan las cantidades para que la búsqueda sea más rápida
static FifoFileControlBlock *fifoFileList = NULL;
static uint64_t fifo_files_count = 0;
static RawFileControlBlock *rawFileList = NULL;
static uint64_t raw_files_count = 0;
static uint64_t nextFileId = 1; // Empieza en 1 porque 0 se usa como "no encontrado" o "error" en varios casos

// Funciones auxiliares
static FifoFileControlBlock* findFifoFile(uint64_t fileId);
static RawFileControlBlock* findRawFile(uint64_t fileId);
static int checkPermissions(InternalFilePermissions permissions, Pid pid, FileAction action);

void initFileSystem() {}

// Función auxiliar para convertir FilePermissions a InternalFilePermissions
// Busca el main del proceso, valida que exista, trae el programa asociado, etc...
// Devuelve 0 si se pudo convertir correctamente, -1 si hubo error
// Deja el resultado en el puntero internalPermissions pasado por referencia
int convertToInternalPermissions(FilePermissions permissions, InternalFilePermissions *internalPermissions) {
    console_log("W: ------> convertToInternalPermissions: Iniciando conversion de permisos");
    printProcessList(); // Para debug, imprimir la lista de procesos

    // me traigo los owners, si existen, traigo el proceso, que sé que es válido y va a incluir el programa
    Pid writing_owner_pid = getProcessGroupMain(permissions.writing_owner);
    Pid reading_owner_pid = getProcessGroupMain(permissions.reading_owner);
    // Pid writing_owner_pid = permissions.writing_owner;
    // Pid reading_owner_pid = permissions.reading_owner;
    log_to_serial("convertToInternalPermissions: Validando owners de permisos");
    log_decimal("convertToInternalPermissions: writing_owner_pid: ", writing_owner_pid);
    log_decimal("convertToInternalPermissions: reading_owner_pid: ", reading_owner_pid);
    log_decimal("convertToInternalPermissions: writing_conditions: ", permissions.writing_conditions);
    log_decimal("convertToInternalPermissions: reading_conditions: ", permissions.reading_conditions);

    if(writing_owner_pid == 0 || reading_owner_pid == 0 || internalPermissions == NULL) {
        // console_log("E: convertToInternalPermissions: Error al convertir permisos, PID invalido o internalPermissions NULL");
        return -1;
    }
    Process writing_owner_process = getProcess(writing_owner_pid);
    Process reading_owner_process = getProcess(reading_owner_pid);

    // Asignar los valores a internalPermissions
    internalPermissions->writing_owner = writing_owner_pid;
    internalPermissions->writing_owner_program = writing_owner_process.program;
    internalPermissions->writing_conditions = permissions.writing_conditions;

    internalPermissions->reading_owner = reading_owner_pid;
    internalPermissions->reading_owner_program = reading_owner_process.program;
    internalPermissions->reading_conditions = permissions.reading_conditions;

    log_to_serial("convertToInternalPermissions: Permisos convertidos correctamente");
    printProcessList(); // Para debug, imprimir la lista de procesos

    return 0;
}


uint64_t createFile(const char *path, FileType type, uint32_t size, FilePermissions permissions) {
    if (path == NULL || strlen(path) >= 256) {
        log_to_serial("E: createFile: path invalido o demasiado largo");
        return 0; // Error: path inválido
    }

    log_to_serial("createFile: Creando archivo");
    log_string("createFile: path: ", path);
    log_decimal("createFile: size: ", size);
    log_decimal("createFile: type: ", type);
    log_decimal("createFile: writing_owner: ", permissions.writing_owner);
    log_decimal("createFile: reading_owner: ", permissions.reading_owner);

    InternalFilePermissions internalPermissions;
    if (convertToInternalPermissions(permissions, &internalPermissions) != 0) {
        log_to_serial("E: createFile: Error al convertir permisos a internos");
        return 0; // Error: permisos inválidos
    }

    uint64_t fileId = nextFileId++;

    if (type == FILE_TYPE_FIFO) {
        FifoFileControlBlock *newFile = (FifoFileControlBlock*)malloc(sizeof(FifoFileControlBlock));
        if (newFile == NULL) {
            return 0; // Error: no hay memoria
        }

        newFile->file.id = fileId;
        strcpy(newFile->file.path, path);
        newFile->file.type = type;
        newFile->file.size = size;
        newFile->readPointer = 0;
        newFile->writePointer = 0;
        newFile->writtenBytes = 0;
        newFile->closed_for_writing = 0;
        newFile->permissions = internalPermissions;
        
        newFile->data = (uint8_t*)malloc(size);
        if (newFile->data == NULL) {
            free(newFile);
            return 0; // Error: no hay memoria
        }

        // Nuevo archivo va al comienzo de la lista
        newFile->next = fifoFileList;
        fifoFileList = newFile;
        fifo_files_count++;

    } else if (type == FILE_TYPE_RAW_DATA) {
        RawFileControlBlock *newFile = (RawFileControlBlock*)malloc(sizeof(RawFileControlBlock));
        if (newFile == NULL) {
            return 0; // Error: no hay memoria
        }

        newFile->file.id = fileId;
        strcpy(newFile->file.path, path);
        newFile->file.type = type;
        newFile->file.size = size;
        newFile->permissions = internalPermissions;
        
        newFile->data = (uint8_t*)malloc(size);
        if (newFile->data == NULL) {
            free(newFile);
            return 0; // Error: no hay memoria
        }

        // Inicializar datos en 0 por las dudas
        for (uint32_t i = 0; i < size; i++) {
            newFile->data[i] = 0;
        }

        newFile->next = rawFileList;
        rawFileList = newFile;
        raw_files_count++;
    } else {
        return 0; // Error: tipo de archivo no soportado
    }

    return fileId;
}

// TODO: emitir el evento
int removeFile(const uint64_t fileId) {
    // Como no sabemos el tipo, buscamos en ambas listas

    // Buscar en archivos FIFO
    FifoFileControlBlock *prevFifo = NULL;
    FifoFileControlBlock *currentFifo = fifoFileList;
    
    while (currentFifo != NULL) {
        if (currentFifo->file.id == fileId) {
            
            // Remover de la lista
            if (prevFifo == NULL) {
                fifoFileList = currentFifo->next;
            } else {
                prevFifo->next = currentFifo->next;
            }
            
            free(currentFifo->data);
            free(currentFifo);
            fifo_files_count--;

            // TODO: emitir un evento de EOF para desbloquear a los procesos que estuvieran esperando en este FIFO (no es necesario por ahora ya que el evento no existe)

            return 0;
        }
        prevFifo = currentFifo;
        currentFifo = currentFifo->next;
    }

    // Buscar en archivos Raw
    RawFileControlBlock *prevRaw = NULL;
    RawFileControlBlock *currentRaw = rawFileList;
    
    while (currentRaw != NULL) {
        if (currentRaw->file.id == fileId) {
            // Remover de la lista
            if (prevRaw == NULL) {
                rawFileList = currentRaw->next;
            } else {
                prevRaw->next = currentRaw->next;
            }
            
            free(currentRaw->data);
            free(currentRaw);
            raw_files_count--;

            // TODO: emitir un evento de EOF para desbloquear a los procesos que estuvieran esperando en este FIFO (no es necesario por ahora ya que el evento no existe)

            return 0;
        }
        prevRaw = currentRaw;
        currentRaw = currentRaw->next;
    }

    return -1; // Error: archivo no encontrado
}

// El pid por ahora no se usa ya que no se lleva un registro de archivos abiertos por proceso
// Pero se deja para que futuras implementaciones no rompan la API
// Devuelve el ID del archivo si se encuentra, 0 si no se encuentra o no se puede abrir (ej. FIFO cerrado para escritura)
// No valida permisos pero sí valida el tipo de archivo, útil ya que al abrir un archivo tenés que saber el tipo porque se leen/escriben de forma diferente
uint64_t openFile(const char *path, Pid pid, FileAction action, FileType type) {
    if(type == FILE_TYPE_FIFO){
        // Archivos FIFO
        FifoFileControlBlock *currentFifo = fifoFileList;
        while (currentFifo != NULL) {
            if (strcmp(currentFifo->file.path, path) == 0) {
                if (action == FILE_WRITE && currentFifo->closed_for_writing) {
                    log_to_serial("E: openFile: archivo FIFO cerrado para escritura");
                    return 0; // Error: archivo cerrado para escritura
                }
                return currentFifo->file.id;
            }
            currentFifo = currentFifo->next;
        }
    } else if(type == FILE_TYPE_RAW_DATA) {
        // Archivos Raw
        RawFileControlBlock *currentRaw = rawFileList;
        while (currentRaw != NULL) {
            if (strcmp(currentRaw->file.path, path) == 0) {
                return currentRaw->file.id;
            }
            currentRaw = currentRaw->next;
        }
    }

    return 0; // Error: archivo no encontrado (o tipo desconocido)
}

// La verdad está de onda esta función, porque como no se lleva un registro de archivos abiertos por proceso, no hace nada
// O sea está meramente para mantener la API y que no se rompa nada después si lo implementamos 
int closeFile(uint64_t fileId, Pid pid) {
    return 0; 
}


// 0 en caso de éxito, -1 si no se pudo cometer la acción
int closeFifoForWriting(uint64_t fileId){
    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile == NULL) {
        log_to_serial("E: closeFifoForWriting: archivo FIFO no encontrado");
        return -1; // Error: archivo no encontrado
    }

    fifoFile->closed_for_writing = 1; // Marcar como cerrado para escritura

    // Si el puntero de lectura alcanzó al de escritura, borrar el archivo
    if (FIFO_EMPTY(fifoFile)) {
        removeFile(fileId); // Esto libera la memoria y elimina el archivo de la lista emitiendo el evento EOF
    }

    return 0; // Éxito
}

void closeAllFifosOfProcess(Pid pid){
    FifoFileControlBlock *currentFifo = fifoFileList;

    while (currentFifo != NULL) {
        // Solo cierro los fifos que son del proceso en cuestión y donde los permisos no sean '*' ni 'p'.
        // Lo de '*' y 'p' es porque con esos permisos aunque el proceso owner muera otro proceso igual puede querer escribir. En este caso solo a mano se puede cerrar.
        if (currentFifo->permissions.writing_owner == pid && currentFifo->permissions.writing_conditions != '*' && currentFifo->permissions.writing_conditions != 'p') {
            closeFifoForWriting(currentFifo->file.id);
        }
        currentFifo = currentFifo->next;
    }
}


// Valida si el proceso tiene permisos para leer/escribir el archivo
// Es booleana, retorna 1 si tiene permisos y 0 si no (si el archivo no existe también retorna 0)
// No se considera si el programa del proceso es "sudo" (o sea tiene permisos globales de acceso a archivos), eso se hace con validatePermissions en el syscallDispatcher como el resto de los permisos globales
int validateFileAccessPermissions(uint64_t fileId, Pid pid, FileAction action) {
    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        return checkPermissions(fifoFile->permissions, pid, action);
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        return checkPermissions(rawFile->permissions, pid, action);
    }

    log_to_serial("E: validateFileAccessPermissions: archivo no encontrado");
    return 0; // Error: archivo no encontrado (se toma como que no tuviera permisos)
}

// Valida si el archivo es del tipo especificado (FIFO o raw_data)
// Útil ya que para leer/escribir no es la misma función para ambos tipos de archivos
int validateFileType(uint64_t fileId, FileType type){
    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        return fifoFile->file.type == type;
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        return rawFile->file.type == type;
    }

    log_to_serial("E: validateFileType: archivo no encontrado");
    return 0; // Error: archivo no encontrado
} 


// ---- Funciones de lectura y escritura ---- //

// Devuelve la cantidad de bytes leídos, o -1 para EOF y -2 para archivo no encontrado 
int64_t readFifo(uint64_t fileId, void *buffer, uint32_t size) {
    FifoFileControlBlock *fileBlock = findFifoFile(fileId);
    if (fileBlock == NULL) {
        log_to_serial("E: readFifo: archivo no encontrado");
        return -2; // Error: archivo no encontrado
    }
    if (FIFO_EMPTY(fileBlock) && fileBlock->closed_for_writing) {
        log_to_serial("E: readFifo: EOF alcanzado");
        removeFile(fileId); // Borra el archivo si se llegó al EOF
        return -1; // EOF alcanzado
    }

    uint32_t bytesRead = 0;
    uint8_t *buf = (uint8_t*)buffer;
        
    // Leer hasta el tamaño solicitado o hasta que no haya más datos (el puntero de lectura alcance al de escritura, es circular)
    while (bytesRead < size && !FIFO_EMPTY(fileBlock)) {
        buf[bytesRead] = fileBlock->data[fileBlock->readPointer];
        ADVANCE_FIFO_POINTER(fileBlock->readPointer, fileBlock->file.size);
        fileBlock->writtenBytes--;
        bytesRead++;
    }    

    return bytesRead;
}

// Devuelve la cantidad de bytes escritos, o -1 para archivo cerrado para escritura y -2 para archivo no encontrado
int64_t writeFifo(uint64_t fileId, void *buffer, uint32_t size) {
    FifoFileControlBlock *fileBlock = findFifoFile(fileId);
    if (fileBlock == NULL) {
        return -2; // Error: archivo no encontrado
    }

    if(fileBlock->closed_for_writing) {
        log_to_serial("E: writeFifo: archivo FIFO cerrado para escritura");
        return -1; // Error: archivo cerrado para escritura
    }

    uint32_t bytesWritten = 0;
    uint8_t *buf = (uint8_t*)buffer;

    while (bytesWritten < size && !FIFO_FULL(fileBlock)) {
        fileBlock->data[fileBlock->writePointer] = buf[bytesWritten];
        ADVANCE_FIFO_POINTER(fileBlock->writePointer, fileBlock->file.size);
        fileBlock->writtenBytes++;
        bytesWritten++;
    }

    return bytesWritten;
}

// Devuelve la cantidad de bytes leídos, o -1 para offset fuera de rango y -2 para archivo no encontrado
int64_t readRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset) {
    RawFileControlBlock *fileBlock = findRawFile(fileId);
    if (fileBlock == NULL) {
        log_to_serial("E: readRaw: archivo no encontrado");
        return -2; // Error: archivo no encontrado
    }

    if (offset >= fileBlock->file.size) {
        log_to_serial("E: readRaw: offset fuera de rango");
        return -1; // Error: offset fuera de rango
    }

    uint32_t maxRead = fileBlock->file.size - offset;
    uint32_t bytesToRead = (size < maxRead) ? size : maxRead;

    uint8_t *buf = (uint8_t*)buffer;
    for (uint32_t i = 0; i < bytesToRead; i++) {
        buf[i] = fileBlock->data[offset + i];
    }

    return bytesToRead;
}

// Devuelve la cantidad de bytes escritos, o -1 para offset fuera de rango y -2 para archivo no encontrado
// No realloca automáticamente
int64_t writeRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset) {
    RawFileControlBlock *fileBlock = findRawFile(fileId);
    if (fileBlock == NULL) {
        return -2; // Error: archivo no encontrado
    }

    if (offset >= fileBlock->file.size) {
        return -1; // Error: offset fuera de rango
    }

    uint32_t maxWrite = fileBlock->file.size - offset;
    uint32_t bytesToWrite = (size < maxWrite) ? size : maxWrite;

    uint8_t *buf = (uint8_t*)buffer;
    for (uint32_t i = 0; i < bytesToWrite; i++) {
        fileBlock->data[offset + i] = buf[i];
    }

    return bytesToWrite;
}

// 0 éxito, -1 error (función interna)
int reallocRawFile(RawFileControlBlock *fileBlock, uint32_t newSize) {
    if (newSize <= fileBlock->file.size) {
        fileBlock->file.size = newSize; // Actualizar el tamaño del archivo
        return 0; // No se necesita reallocar
    }

    uint8_t *newData = (uint8_t*)malloc(newSize);
    if (newData == NULL) {
        log_to_serial("E: reallocRawFile: no hay memoria para alocar mas datos");
        return -1; // Error: no hay memoria
    }

    // Copiar los datos existentes al nuevo buffer
    for (uint32_t i = 0; i < fileBlock->file.size; i++) {
        newData[i] = fileBlock->data[i];
    }
    // Llenar con ceros el resto del nuevo buffer
    for (uint32_t i = fileBlock->file.size; i < newSize; i++) {
        newData[i] = 0;
    }

    // Actualizar el puntero de datos del archivo
    free(fileBlock->data); // Liberar el buffer antiguo
    fileBlock->data = newData;
    fileBlock->file.size = newSize; // Actualizar el tamaño del archivo
    return 0; // Éxito
}

// Escribe en un archivo raw, reallocando si es necesario
// Devuelve la cantidad de bytes escritos, o -1 para error de reallocación y -2 para archivo no encontrado
int64_t writeRawWithRealloc(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset) {
    RawFileControlBlock *fileBlock = findRawFile(fileId);
    if (fileBlock == NULL) {
        return -2; // Error: archivo no encontrado
    }

    if (offset + size > fileBlock->file.size) {
        if (reallocRawFile(fileBlock, offset + size) != 0) {
            return -1; // Error: no se pudo reallocar el archivo
        }
    }

    return writeRaw(fileId, buffer, size, offset); // Llamar a la función de escritura normal
}


File getFileById(uint64_t fileId) {
    File emptyFile = {.id = 0};
    
    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        return fifoFile->file;
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        return rawFile->file;
    }

    return emptyFile; // Archivo no encontrado
}

// Devuelve una lista de IDs de todos los archivos, ordenada por path
// La lista es dinámica, es tarea de quien llama a la función liberar la memoria
// Devuelve NULL si no se encontraron archivos o si hubo un error
// La lista termina con un 0 para indicar el final
// La verdad podría ser más eficiente si guardo los archivos en orden directamente, pero bueno... quedará pendiente
uint64_t * listFiles() {
    uint32_t totalFiles = fifo_files_count + raw_files_count;

    if (totalFiles == 0) {
        return NULL;
    }
    
    // Crear array temporal para ordenar por path
    typedef struct {
        uint64_t id;
        char path[256]; // inicialmente lo programé siendo directo un puntero al path del FCB, para no usar strcpy, 
                        // pero por las dudas lo copio porque si se borra un archivo en el medio de esto (si queremos hacer que PinkOS sea multi-threaded por ejemplo)
                        // habría problemas porque después para el ordenamiento estaría leyendo un puntero a memoria liberada
                        // igual medio que esta sería el menor de los problemas si queremos hacer PinkOS multi-threaded me parece...
    } FileEntry;

    FileEntry *fileEntries = (FileEntry*)malloc(sizeof(FileEntry) * totalFiles);
    if (fileEntries == NULL) {
        return NULL; // Error de memoria
    }
    
    // Llenar array con archivos FIFO
    uint32_t index = 0;
    FifoFileControlBlock * currentFifo = fifoFileList;
    while (currentFifo != NULL) {
        fileEntries[index].id = currentFifo->file.id;
        strcpy(fileEntries[index].path, currentFifo->file.path);
        index++;
        currentFifo = currentFifo->next;
    }
    
    // Llenar array con archivos Raw
    RawFileControlBlock * currentRaw = rawFileList;
    while (currentRaw != NULL) {
        fileEntries[index].id = currentRaw->file.id;
        strcpy(fileEntries[index].path, currentRaw->file.path);
        index++;
        currentRaw = currentRaw->next;
    }
    
    // Ordenar por path usando bubble sort (no es lo más eficiente pero es simple y funca)
    for (uint32_t i = 0; i < totalFiles - 1; i++) {
        for (uint32_t j = 0; j < totalFiles - i - 1; j++) {
            if (strcmp(fileEntries[j].path, fileEntries[j + 1].path) > 0) {
                // Intercambiar
                FileEntry temp = fileEntries[j];
                fileEntries[j] = fileEntries[j + 1];
                fileEntries[j + 1] = temp;
            }
        }
    }
    
    // Crear el array de resultados (null terminated)
    uint64_t *result = (uint64_t*)malloc(sizeof(uint64_t) * (totalFiles + 1));
    if (result == NULL) {
        free(fileEntries);
        return NULL; // Error de memoria
    }
    
    // Copiar los IDs al array de resultados
    for (uint32_t i = 0; i < totalFiles; i++) {
        result[i] = fileEntries[i].id;
    }
    result[totalFiles] = 0; // termino con null
    
    free(fileEntries);
    return result;
}


// El pid que se pasa se usa nomás para validar que el proceso tenga permisos para cambiar los permisos del archivo
// Solo alguien del grupo del owner puede cambiar los permisos
// Pid 0 se toma como modo kernel, o sea que puede setear permisos de cualquier archivo
int setFilePermissions(uint64_t fileId, Pid pid, FilePermissions permissions) {
    console_log("setFilePermissions: Cambiando permisos del archivo");
    printProcessList(); // Para debug, imprimir la lista de procesos

    InternalFilePermissions internalPermissions;
    if (convertToInternalPermissions(permissions, &internalPermissions) != 0) {
        // console_log("E: setFilePermissions: Error al convertir permisos a internos");
        return -1; // Error: permisos inválidos/mal formados
    }
    // console_log("setFilePermissions: Convertidos permisos a internos correctamente");
    printProcessList(); // Para debug, imprimir la lista de procesos

    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    // console_log("setFilePermissions: Buscando archivo FIFO");
        printProcessList(); // Para debug, imprimir la lista de procesos

    if (fifoFile != NULL) {
        if(pid && isSameProcessGroup(fifoFile->permissions.writing_owner, pid)) {
            // console_log("E: setFilePermissions: Error: sin permisos para cambiar los permisos");
            return -1; // Error: sin permisos para cambiar los permisos
        }
        fifoFile->permissions = internalPermissions;
        // console_log("setFilePermissions: Permisos cambiados correctamente, archivo FIFO encontrado");
        printProcessList(); // Para debug, imprimir la lista de procesos
        return 0; // Éxito
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        if(pid && isSameProcessGroup(rawFile->permissions.writing_owner, pid)) {
            // console_log("E: setFilePermissions: Error: sin permisos para cambiar los permisos");
            return -1; // Error: sin permisos para cambiar los permisos
        }
        rawFile->permissions = internalPermissions;
        return 0; // Éxito
    }
    // console_log("E: setFilePermissions: Error: archivo no encontrado");

    return -1; // Error: archivo no encontrado
}

FilePermissions getFilePermissions(uint64_t fileId) {
    FilePermissions emptyPermissions = {0};

    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        FilePermissions permissions;
        permissions.writing_owner = fifoFile->permissions.writing_owner;
        permissions.writing_conditions = fifoFile->permissions.writing_conditions;
        permissions.reading_owner = fifoFile->permissions.reading_owner;
        permissions.reading_conditions = fifoFile->permissions.reading_conditions;
        return permissions;
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        FilePermissions permissions;
        permissions.writing_owner = rawFile->permissions.writing_owner;
        permissions.writing_conditions = rawFile->permissions.writing_conditions;
        permissions.reading_owner = rawFile->permissions.reading_owner;
        permissions.reading_conditions = rawFile->permissions.reading_conditions;
        return permissions;
    }

    return emptyPermissions; // Archivo no encontrado
}

int setFilePath(uint64_t fileId, const char *newPath) {
    if (newPath == NULL || strlen(newPath) >= 256) {
        return -1; // Error: path inválido
    }

    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        strcpy(fifoFile->file.path, newPath);
        return 0;
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        strcpy(rawFile->file.path, newPath);
        return 0;
    }

    return -1; // Error: archivo no encontrado
}



static FifoFileControlBlock* findFifoFile(uint64_t fileId) {
    FifoFileControlBlock *current = fifoFileList;
    while (current != NULL) {
        if (current->file.id == fileId) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static RawFileControlBlock* findRawFile(uint64_t fileId) {
    RawFileControlBlock *current = rawFileList;
    while (current != NULL) {
        if (current->file.id == fileId) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}


// Función booleana que en base a una struct de permisos, indica si un pid particular tiene permisos para realizar una acción dada
// No se considera si el programa del proceso es "sudo" (o sea tiene permisos globales de acceso a archivos), eso se hace con validatePermissions en el syscallDispatcher como el resto de los permisos globales
// Devuelve 1 si tiene permisos, 0 si no
//* Es interna, la función pública es validateFileAccessPermissions y recibe un archivo, no un struct de permisos
static int checkPermissions(InternalFilePermissions permissions, Pid pid, FileAction action) {
    // Determinar qué permisos verificar según la acción
    Pid owner;
    Program program;
    char condition;
    
    if(action == FILE_REMOVE){
        // Solo el owner de escritura y su grupo puede eliminar el archivo
        return isSameProcessGroup(pid, permissions.writing_owner);
    } else if (action == FILE_WRITE) {
        owner = permissions.writing_owner;
        program = permissions.writing_owner_program;
        condition = permissions.writing_conditions;
    } else if (action == FILE_READ) {
        owner = permissions.reading_owner;
        program = permissions.reading_owner_program;
        condition = permissions.reading_conditions;
    } else if (action == FILE_READ_WRITE) {
        return checkPermissions(permissions, pid, FILE_READ) && checkPermissions(permissions, pid, FILE_WRITE);
    } else {
        return 0; // Acción no válida, no tiene permisos
    }

    switch (condition) {
        case '*': // Cualquiera puede acceder
            return 1;
        case '-': // Nadie puede acceder
            return 0;
        case '.': // Solo el owner y su grupo
            return isSameProcessGroup(pid, owner);
        case '+': // Solo el grupo del owner y sus descendientes
            return isDescendantOf(pid, owner); // no hace falta traer el main del owner porque un thread siempre crea archivos a nombre del main del grupo, la validación está en convertToInternalPermissions 
        case 'p': // Procesos del mismo programa 
            Process requesting_process = getProcess(pid); 
            return (requesting_process.pid != 0 && requesting_process.program.command == program.command);
        default:
            return 0; // Condición desconocida
    }
}


