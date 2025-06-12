#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>


// ----- Forward declarations -----
typedef struct File File;

// ----- Basic Types -----
typedef uint64_t Pid; // 64 bits para que puedas crear 40.000 procesos por segundo hasta que el sol muera sin tener que reiniciar el sistema

// ----- File System Types -----

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
// '*' para todos, '-' para nadie, '.' para el grupo del proceso owner, '+' para el grupo del proceso actual y sus descendientes y 'p' para todos los procesos del mismo programa que el owner
//      -> El grupo de un proceso son todos los procesos que comparten el mismo proceso main (es decir, un proceso y sus threads)
//      -> Los descendientes son todos los procesos que en el arbol de procesos están por debajo del proceso main del proceso actual
//      -> El permiso "nadie" es útil por ejemplo para crear archivos solo lectura o archivos que solo puede usar el kernel
//      -> Tanto un programa con permisos root y el kernel mismo pueden leer y escribir en cualquier archivo, sin importar las condiciones
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

// ----- Process Manager Types -----

//TODO: args deberían ser void* para poder pasar cualquier cosa, cada programa sabrá qué recibe (normalmente sería un string pero en el caso de threads podría ser un struct con más información)
typedef void (*ProgramEntry)(char*);

#define SMALL_TEXT_SIZE 64
#define MEDIUM_TEXT_SIZE 56

// CRI (Context recovery info) -> nos re inventamos el nombre el cuatri pasado lol, cosas de querer implementar pseudo procesos antes de cursar SO
typedef struct {
    uint64_t ss;      // Stack Segment
    uint64_t rsp;     // Stack Pointer
    uint64_t rflags;  // Flags Register
    uint64_t cs;      // Code Segment
    uint64_t rip;     // Instruction Pointer
} InterruptStackFrame;

// typedef struct {
//     char command[SMALL_TEXT_SIZE];
//     char name[SMALL_TEXT_SIZE];
//     ProgramEntry entry;
//     uint32_t permissions;
//     char help[MEDIUM_TEXT_SIZE];  // A very brief description
//     char* description;  // All the information about the command (deprecated by the man page file)
//     File man_page; // A publically readable but unmodifiable file with the program's manual
// } Program;

typedef struct {
    char* command;
    char* name;
    ProgramEntry entry;
    uint32_t permissions;
    char* help;         // This is the help command (a very brief description)
    char* description;  // All the information about the command
} Program;

typedef enum {
    PRIORITY_LOW,
    PRIORITY_NORMAL,
    PRIORITY_HIGH,
} Priority;

typedef enum ProcessState {
    PROCESS_STATE_NEW,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_READY,
    PROCESS_STATE_WAITING,
    PROCESS_STATE_TERMINATED,
} ProcessState;

typedef enum {
    PROCESS_TYPE_MAIN, // Proceso normal que ejecuta un programa
    PROCESS_TYPE_THREAD,  // Thread que comparte el mismo espacio de memoria que su padre
} ProcessType;

// TODO: agregar parent
typedef struct Process {
    Pid pid;
    ProcessType type;
    ProcessState state;
    Priority priority;  // Prioridad del proceso, 0 es baja, 1 es normal, 2 es alta
    Program program;    // Programa asociado al proceso
} Process;


#endif // TYPES_H