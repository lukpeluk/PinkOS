#include <stdint.h>
#include <stdlib.h>
#include <fileSystem/fileSystem.h>
#include <memoryManager/memoryManager.h>
#include <lib.h>

// A futuro podría guardar una lista de quién lo abrió para permitir cosas como no permitir escritura mientras alguien lo está leyendo
// Por ahora igual los procesos pueden usar Mutexes para sincronizarse entre ellos así que no es prioritario
typedef struct FifoFileControlBlock {
    File file;        // Información del archivo
    uint8_t *data;    // Puntero a los datos del archivo
    uint64_t readPointer;  // Puntero de lectura FIFO
    uint64_t writePointer; // Puntero de escritura FIFO
    struct FifoFileControlBlock *next; // Linked list de archivos (para mejor eficiencia podría ser un árbol pero bueno, por ahora es suficiente)
} FifoFileControlBlock;

typedef struct RawFileControlBlock {
    File file;        // Información del archivo
    uint8_t *data;    // Puntero a los datos del archivo
    struct RawFileControlBlock *next; // Linked list de archivos (para mejor eficiencia podría ser un árbol pero bueno, por ahora es suficiente)
} RawFileControlBlock;

// Variables globales del filesystem
static FifoFileControlBlock *fifoFileList = NULL;
static RawFileControlBlock *rawFileList = NULL;
static uint64_t nextFileId = 1;

// Funciones auxiliares
static int strcmp_impl(const char *str1, const char *str2);
static void strcpy_impl(char *dest, const char *src);
static int strlen_impl(const char *str);
static FifoFileControlBlock* findFifoFile(uint64_t fileId);
static RawFileControlBlock* findRawFile(uint64_t fileId);
static int checkPermissions(FilePermissions permissions, Pid pid, FileAction action);

void initFileSystem() {
    fifoFileList = NULL;
    rawFileList = NULL;
    nextFileId = 1;
}

uint64_t createFile(const char *path, FileType type, uint32_t size, FilePermissions permissions) {
    if (path == NULL || strlen_impl(path) >= 256) {
        return 0; // Error: path inválido
    }

    uint64_t fileId = nextFileId++;

    if (type == FILE_TYPE_FIFO) {
        FifoFileControlBlock *newFile = (FifoFileControlBlock*)malloc(sizeof(FifoFileControlBlock));
        if (newFile == NULL) {
            return 0; // Error: no hay memoria
        }

        newFile->file.id = fileId;
        strcpy_impl(newFile->file.path, path);
        newFile->file.type = type;
        newFile->file.size = size;
        newFile->file.permissions = permissions;
        newFile->readPointer = 0;
        newFile->writePointer = 0;
        
        newFile->data = (uint8_t*)malloc(size);
        if (newFile->data == NULL) {
            free(newFile);
            return 0; // Error: no hay memoria
        }

        newFile->next = fifoFileList;
        fifoFileList = newFile;
    } else {
        RawFileControlBlock *newFile = (RawFileControlBlock*)malloc(sizeof(RawFileControlBlock));
        if (newFile == NULL) {
            return 0; // Error: no hay memoria
        }

        newFile->file.id = fileId;
        strcpy_impl(newFile->file.path, path);
        newFile->file.type = type;
        newFile->file.size = size;
        newFile->file.permissions = permissions;
        
        newFile->data = (uint8_t*)malloc(size);
        if (newFile->data == NULL) {
            free(newFile);
            return 0; // Error: no hay memoria
        }

        // Inicializar datos en 0
        for (uint32_t i = 0; i < size; i++) {
            newFile->data[i] = 0;
        }

        newFile->next = rawFileList;
        rawFileList = newFile;
    }

    return fileId;
}

int removeFile(const uint64_t fileId, Pid pid) {
    // Buscar en archivos FIFO
    FifoFileControlBlock *prevFifo = NULL;
    FifoFileControlBlock *currentFifo = fifoFileList;
    
    while (currentFifo != NULL) {
        if (currentFifo->file.id == fileId) {
            // Verificar permisos (solo el owner puede eliminar)
            if (currentFifo->file.permissions.writing_owner != pid) {
                return -1; // Error: sin permisos
            }
            
            // Remover de la lista
            if (prevFifo == NULL) {
                fifoFileList = currentFifo->next;
            } else {
                prevFifo->next = currentFifo->next;
            }
            
            free(currentFifo->data);
            free(currentFifo);
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
            // Verificar permisos (solo el owner puede eliminar)
            if (currentRaw->file.permissions.writing_owner != pid) {
                return -1; // Error: sin permisos
            }
            
            // Remover de la lista
            if (prevRaw == NULL) {
                rawFileList = currentRaw->next;
            } else {
                prevRaw->next = currentRaw->next;
            }
            
            free(currentRaw->data);
            free(currentRaw);
            return 0;
        }
        prevRaw = currentRaw;
        currentRaw = currentRaw->next;
    }

    return -1; // Error: archivo no encontrado
}

uint64_t openFile(const char *path, Pid pid, FileAction action) {
    // Buscar en archivos FIFO
    FifoFileControlBlock *currentFifo = fifoFileList;
    while (currentFifo != NULL) {
        if (strcmp_impl(currentFifo->file.path, path) == 0) {
            if (validateFileAccessPermissions(currentFifo->file.id, pid, action) == 0) {
                return currentFifo->file.id;
            }
            return 0; // Error: sin permisos
        }
        currentFifo = currentFifo->next;
    }

    // Buscar en archivos Raw
    RawFileControlBlock *currentRaw = rawFileList;
    while (currentRaw != NULL) {
        if (strcmp_impl(currentRaw->file.path, path) == 0) {
            if (validateFileAccessPermissions(currentRaw->file.id, pid, action) == 0) {
                return currentRaw->file.id;
            }
            return 0; // Error: sin permisos
        }
        currentRaw = currentRaw->next;
    }

    return 0; // Error: archivo no encontrado
}

int closeFile(uint64_t fileId, Pid pid) {
    // Por ahora solo validamos que el archivo existe
    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        return 0;
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        return 0;
    }

    return -1; // Error: archivo no encontrado
}

int validateFileAccessPermissions(uint64_t fileId, Pid pid, FileAction action) {
    // Buscar el archivo
    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        return checkPermissions(fifoFile->file.permissions, pid, action);
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        return checkPermissions(rawFile->file.permissions, pid, action);
    }

    return -1; // Error: archivo no encontrado
}

uint32_t readFifo(uint64_t fileId, void *buffer, uint32_t size) {
    FifoFileControlBlock *file = findFifoFile(fileId);
    if (file == NULL) {
        return 0; // Error: archivo no encontrado
    }

    uint32_t bytesRead = 0;
    uint8_t *buf = (uint8_t*)buffer;

    while (bytesRead < size && file->readPointer != file->writePointer) {
        buf[bytesRead] = file->data[file->readPointer];
        file->readPointer = (file->readPointer + 1) % file->file.size;
        bytesRead++;
    }

    return bytesRead;
}

uint32_t writeFifo(uint64_t fileId, void *buffer, uint32_t size) {
    FifoFileControlBlock *file = findFifoFile(fileId);
    if (file == NULL) {
        return 0; // Error: archivo no encontrado
    }

    uint32_t bytesWritten = 0;
    uint8_t *buf = (uint8_t*)buffer;

    while (bytesWritten < size) {
        uint64_t nextWritePointer = (file->writePointer + 1) % file->file.size;
        if (nextWritePointer == file->readPointer) {
            // FIFO lleno
            break;
        }

        file->data[file->writePointer] = buf[bytesWritten];
        file->writePointer = nextWritePointer;
        bytesWritten++;
    }

    return bytesWritten;
}

uint32_t readRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset) {
    RawFileControlBlock *file = findRawFile(fileId);
    if (file == NULL) {
        return 0; // Error: archivo no encontrado
    }

    if (offset >= file->file.size) {
        return 0; // Error: offset fuera de rango
    }

    uint32_t maxRead = file->file.size - offset;
    uint32_t bytesToRead = (size < maxRead) ? size : maxRead;

    uint8_t *buf = (uint8_t*)buffer;
    for (uint32_t i = 0; i < bytesToRead; i++) {
        buf[i] = file->data[offset + i];
    }

    return bytesToRead;
}

uint32_t writeRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset) {
    RawFileControlBlock *file = findRawFile(fileId);
    if (file == NULL) {
        return 0; // Error: archivo no encontrado
    }

    if (offset >= file->file.size) {
        return 0; // Error: offset fuera de rango
    }

    uint32_t maxWrite = file->file.size - offset;
    uint32_t bytesToWrite = (size < maxWrite) ? size : maxWrite;

    uint8_t *buf = (uint8_t*)buffer;
    for (uint32_t i = 0; i < bytesToWrite; i++) {
        file->data[offset + i] = buf[i];
    }

    return bytesToWrite;
}

uint64_t * listFiles(char * path) {
    // Contar archivos que coincidan con el path
    int count = 0;
    FifoFileControlBlock *currentFifo = fifoFileList;
    while (currentFifo != NULL) {
        if (strcmp_impl(currentFifo->file.path, path) == 0) {
            count++;
        }
        currentFifo = currentFifo->next;
    }

    RawFileControlBlock *currentRaw = rawFileList;
    while (currentRaw != NULL) {
        if (strcmp_impl(currentRaw->file.path, path) == 0) {
            count++;
        }
        currentRaw = currentRaw->next;
    }

    // Crear array de IDs (null terminated)
    uint64_t *result = (uint64_t*)malloc((count + 1) * sizeof(uint64_t));
    if (result == NULL) {
        return NULL;
    }

    int index = 0;
    currentFifo = fifoFileList;
    while (currentFifo != NULL) {
        if (strcmp_impl(currentFifo->file.path, path) == 0) {
            result[index++] = currentFifo->file.id;
        }
        currentFifo = currentFifo->next;
    }

    currentRaw = rawFileList;
    while (currentRaw != NULL) {
        if (strcmp_impl(currentRaw->file.path, path) == 0) {
            result[index++] = currentRaw->file.id;
        }
        currentRaw = currentRaw->next;
    }

    result[count] = 0; // Null terminator
    return result;
}

File getFileById(uint64_t fileId) {
    File emptyFile = {0};
    
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

int setFilePermissions(uint64_t fileId, Pid pid, FilePermissions permissions) {
    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        if (fifoFile->file.permissions.writing_owner == pid) {
            fifoFile->file.permissions = permissions;
            return 0;
        }
        return -1; // Error: sin permisos
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        if (rawFile->file.permissions.writing_owner == pid) {
            rawFile->file.permissions = permissions;
            return 0;
        }
        return -1; // Error: sin permisos
    }

    return -1; // Error: archivo no encontrado
}

int setFilePath(uint64_t fileId, Pid pid, const char *newPath) {
    if (newPath == NULL || strlen_impl(newPath) >= 256) {
        return -1; // Error: path inválido
    }

    FifoFileControlBlock *fifoFile = findFifoFile(fileId);
    if (fifoFile != NULL) {
        if (fifoFile->file.permissions.writing_owner == pid) {
            strcpy_impl(fifoFile->file.path, newPath);
            return 0;
        }
        return -1; // Error: sin permisos
    }

    RawFileControlBlock *rawFile = findRawFile(fileId);
    if (rawFile != NULL) {
        if (rawFile->file.permissions.writing_owner == pid) {
            strcpy_impl(rawFile->file.path, newPath);
            return 0;
        }
        return -1; // Error: sin permisos
    }

    return -1; // Error: archivo no encontrado
}

// Funciones auxiliares

static int strcmp_impl(const char *str1, const char *str2) {
    while (*str1 && *str2 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

static void strcpy_impl(char *dest, const char *src) {
    while (*src) {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';
}

static int strlen_impl(const char *str) {
    int len = 0;
    while (*str) {
        len++;
        str++;
    }
    return len;
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

static int checkPermissions(FilePermissions permissions, Pid pid, FileAction action) {
    // Determinar qué permisos verificar según la acción
    Pid owner;
    char condition;
    
    if (action == FILE_WRITE) {
        owner = permissions.writing_owner;
        condition = permissions.writing_conditions;
    } else {
        owner = permissions.reading_owner;
        condition = permissions.reading_conditions;
    }

    switch (condition) {
        case '*': // Todos pueden acceder
            return 0;
        case '-': // Nadie puede acceder
            return -1;
        case '.': // Solo el owner
            return (pid == owner) ? 0 : -1;
        case '+': // Owner y sus hijos/threads (simplificado: mismo owner)
            return (pid == owner) ? 0 : -1;
        case 'p': // Procesos del mismo programa (simplificado: mismo owner)
            return (pid == owner) ? 0 : -1;
        default:
            return -1; // Condición desconocida
    }
}


