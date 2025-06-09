#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <types.h>

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