#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>

typedef enum {
    FILE_TYPE_FIFO = 0,
    FILE_TYPE_RAW_DATA = 1,
} FileType;

// "*" para todos, <PID> para un proceso específico, +<PID> para un proceso y sus hijos/threads y <command> para un programa específico
typedef char * FilePermissions;

typedef struct {
    uint64_t id;
    char *path;         // Ruta al archivo incluyendo el nombre
    FileType type;      // Tipo de archivo, si es FIFO, raw_data, etc. Afecta a cómo se lee y escribe el archivo
    uint32_t size;      // Tamaño del archivo en bytes
    uint8_t *data;      // Puntero a los datos del archivo
    // Quién puede leer y quién puede escribir
    FilePermissions readPermissions; 
    FilePermissions writePermissions; 
} File;

void initFileSystem();

uint64_t createFile(const char *path, FilePermissions readPermissions, FilePermissions writePermissions, FileType type, uint32_t size);
int removeFile(const uint64_t fileId);

uint64_t openFile(const char *path); // Abre un archivo y devuelve su ID, 0 si no se pudo abrir

// Las funciones de lectura/escritura devuelven los bytes que en efecto se leyeron/escribieron
uint32_t readFifo(uint64_t fileId, void *buffer, uint32_t size);
uint32_t writeFifo(uint64_t fileId, void *buffer, uint32_t size);

uint32_t readRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset);
uint32_t writeRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset);

char * getFilePath(uint64_t fileId); // Devuelve la ruta al archivo incluyendo el nombre
FileType getFileType(uint64_t fileId);
uint64_t getFileSize(uint64_t fileId);
FilePermissions getFileReadPermissions(uint64_t fileId);
FilePermissions getFileWritePermissions(uint64_t fileId);

uint64_t * listFiles(char * path); // devuelve los IDs de los archivos en el directorio especificado, arreglo null terminated

void listFilesByPermissions(FilePermissions permissions); // Lista archivos según permisos
void setFilePermissions(uint64_t fileId, FilePermissions readPermissions, FilePermissions writePermissions);
void setFileType(uint64_t fileId, FileType type); // Cambia el tipo de archivo
void setFileSize(uint64_t fileId, uint32_t size); // Cambia el tamaño del archivo
void setFileData(uint64_t fileId, uint8_t *data, uint32_t size); // Cambia los datos del archivo

#endif