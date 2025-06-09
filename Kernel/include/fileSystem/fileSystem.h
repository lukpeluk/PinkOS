#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <processManager/scheduler.h>
#include <processManager/processState.h>

typedef enum {
    FILE_TYPE_FIFO = 0,
    FILE_TYPE_RAW_DATA = 1,
    FILE_TYPE_SONG = 2,
    FILE_TYPE_PROGRAM = 3,
} FileType;

typedef enum {
    FILE_READ = 'r',  
    FILE_WRITE = 'w', 
    FILE_READ_WRITE = 'a',
} FileAction;

// Quién puede leer y escribir en un archivo, no necesariamente quien puede leer es quien puede escribir
// (especialmente en el caso de FIFO (pipes), donde al ser consumible no quiero que quien escribe pueda leer)
// Las condiciones de lectura/escritura son:
// '*' para todos, '-' para nadie, '.' para el proceso owner, '+' para el proceso owner y sus hijos/threads y 'p' para todos los procesos del mismo programa que el owner
typedef struct FilePermissions{
    Pid writing_owner;
    char writing_conditions;
    Pid reading_owner;
    char reading_conditions;
} FilePermissions;

typedef struct File{
    uint64_t id;    // ID único del archivo
    char path[256]; // Ruta al archivo incluyendo el nombre
    FileType type;  // Tipo de archivo, si es FIFO, raw_data, etc. Afecta a cómo se lee y escribe el archivo
    uint32_t size;  // Tamaño del archivo en bytes
    FilePermissions permissions; // Quién puede leer/escribir el archivo
} File;

void initFileSystem();

// Crea un archivo nuevo, devuelve su ID o 0 si no se pudo crear
uint64_t createFile(const char *path, FileType type, uint32_t size, FilePermissions permissions);
// 0 en caso de éxito, -1 si no se pudo crear el archivo
int removeFile(const uint64_t fileId, Pid pid);

// "Abre" un archivo (devuelve su ID, 0 si no se pudo abrir)
// Por ahora no tiene tanta utilidad más que para saber el id de un archivo/validar permisos, pero si se trackea quién abrió qué
uint64_t openFile(const char *path, Pid pid, FileAction action); 
// Cierra un archivo, devuelve 0 en caso de éxito (por ahora es un return 0 porque no mantenemos lógica de archivos abiertos)
int closeFile(uint64_t fileId, Pid pid); 

int validateFileAccessPermissions(uint64_t fileId, Pid pid, FileAction action); // Valida si el proceso tiene permisos para leer/escribir el archivo

// Las funciones de lectura/escritura devuelven la cantidad de bytes que en efecto se leyeron/escribieron
uint32_t readFifo(uint64_t fileId, void *buffer, uint32_t size);
uint32_t writeFifo(uint64_t fileId, void *buffer, uint32_t size);

uint32_t readRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset);
uint32_t writeRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset);


uint64_t * listFiles(char * path); // devuelve los IDs de los archivos en el directorio especificado, arreglo null terminated
File getFileById(uint64_t fileId);

// Solo el owner de la respectiva acción o una syscall corriendo en modo kernel pueden cambiar los permisos de un archivo
// Retorna 0 si se pudo cambiar, -1 si no se pudo cambiar 
int setFilePermissions(uint64_t fileId, Pid pid, FilePermissions permissions);

// Cambia la ruta del archivo, devuelve 0 si se pudo cambiar, -1 si no se pudo cambiar
int setFilePath(uint64_t fileId, Pid pid, const char *newPath); 

#endif