#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <stdint.h>
#include <types.h>

void initFileSystem();

/* DOCS DEL FILESYSTEM
    * Los archivos tienen un identificador único (ID) y una ruta (path)
        * Cualquier acción sobre un archivo se hace a través de su ID
        * Para conseguir el ID de un archivo, hay que abrirlo con openFile, pasando la ruta, quién sos, y para qué acción lo querés abrir
        * Los IDs son globales, es decir, no hay file descriptors por proceso, sino que cada archivo tiene un ID único en todo el sistema
            * De esa manera se pueden hacer pipes fácilmente, con compartir el ID del archivo entre procesos
            * Para restringir quién accede se usan los permisos de lectura/escritura
                - De esa forma, el pipe se hace como corresponde: un proceso escribe en el FIFO y otro lo lee, sin que el que escribe pueda leerlo o viseversa
         
    * Los archivos pueden ser de tipo FIFO (más que nada para pipes) o raw_data
    * La diferencia entre FIFO y raw es que FIFO es un buffer circular y es consumible (leer "destruye" la información) y raw es un buffer lineal de un tamaño dado donde se puede leer y escribir en cualquier parte sin perder datos
        * Otra particularidad de los FIFO es que tienen EOF, o sea, si el proceso que escribe muere o decide que no va a escribir más, el FIFO se cierra para escritura y al llegar al final del buffer se devuelve EOF y se borra el archivo
        * Por ahora no hay más tipos de archivos, pero se pueden agregar fácilmente
        * Los raw_data pueden guardar cualquier tipo de dato, como imágenes, canciones, etc.

    * Los archivos tienen permisos de lectura y escritura, que pueden ser diferentes (el proceso con permisos de escribir puede no poder leer y a la inversa)
        * Los permisos posibles son los siguientes:
        -> '*' para todos, '-' para nadie, '.' para el grupo del proceso owner, '+' para el grupo del proceso actual y sus descendientes y 'p' para todos los procesos del mismo programa que el owner
            - El grupo de un proceso son todos los procesos que comparten el mismo proceso main (es decir, un proceso y sus threads)
            - Los descendientes son todos los procesos que en el arbol de procesos están por debajo del proceso main del proceso actual
            - El permiso "nadie" es útil por ejemplo para crear archivos solo lectura o archivos que solo puede usar el kernel
            - Tanto un programa con permisos root y el kernel mismo pueden leer y escribir en cualquier archivo, sin importar las condiciones
        * Para setear permisos, se pasa una estructura FilePermissions básica que solo tiene el pid del owner y las condiciones
            -> pero internamente se guarda algo más complejo en una estructura interna, ya que se guarda el programa del owner y además el owner siempre será el proceso main del grupo de procesos por si el thread muere

    * Sobre EOF/cerrar escritura en archivos FIFO:
        * Cuando un archivo se cierra para escritura, significa que ya nada nuevo va a escribirse en él
            -> Entonces si cuando se cierra este no tiene nada (r == w), se borra el archivo
        * Cuando un proceso muere, se cierran los archivos FIFO que lo tengan como owner de escritura (si los permisos no son * o p)
        * Si el proceso owner de escritura decide, puede cerrar un archivo a mano aunque no muera
        * Cuando se borra un archivo, se emite un evento que es como si ocurriera un EOF en el mismo, para desbloquear a los procesos que estuvieran esperando

    * Si un proceso no borra los archivos que crea, (excepto en el caso de fifos), estos quedan para siempre hasta que el usuario los borre a mano, al igual que ocurre en un sistema cualquiera (no es como la memoria alocada que se libera sola)
    * La validación de los permisos de lectura/escritura la debe hacer el kernel antes de usar la función read/write, no se valida dentro de la función ya que el kernel si quiere puede acceder a todo
        * Para la validación de permisos, el filesystem provee una función validateFileAccessPermissions que recibe el ID del archivo, el PID del proceso y la acción que se quiere realizar (lectura/escritura) 
        * Para poder validar los permisos, el filesystem usa funciones del scheduler para saber si el proceso pertenece al grupo o descendientes del owner del archivo
*/



// Crea un archivo nuevo, devuelve su ID o 0 si no se pudo crear
uint64_t createFile(const char *path, FileType type, uint32_t size, FilePermissions permissions);
// 0 en caso de éxito, -1 si no se pudo borrar el archivo
int removeFile(const uint64_t fileId);

// "Abre" un archivo (devuelve su ID, 0 si no se pudo abrir)
// Por ahora no tiene tanta utilidad más que para saber el id de un archivo/validar permisos, pero si se trackea quién abrió qué
uint64_t openFile(const char *path, Pid pid, FileAction action, FileType type); 
// Cierra un archivo, devuelve 0 en caso de éxito (por ahora es un return 0 porque no mantenemos lógica de archivos abiertos)
int closeFile(uint64_t fileId, Pid pid); 

int closeFifoForWriting(uint64_t fileId); // 0 en caso de éxito, -1 si no se pudo cometer la acción
void closeAllFifosOfProcess(Pid pid);

// Valida si el proceso tiene permisos para leer/escribir el archivo
// Es booleana, retorna 1 si tiene permisos y 0 si no (si el archivo no existe también retorna 0)
// No se considera si el programa del proceso es "sudo" (o sea tiene permisos globales de acceso a archivos), eso se hace con validatePermissions en el syscallDispatcher como el resto de los permisos globales
int validateFileAccessPermissions(uint64_t fileId, Pid pid, FileAction action); 

int validateFileType(uint64_t fileId, FileType type); // Valida si el archivo es del tipo especificado (FIFO o raw_data)

/* 
 * Las funciones de lectura/escritura devuelven en caso >= 0, la cantidad de bytes que en efecto se leyeron/escribieron
 * Si la respuesta es < 0, significa que hubo un error o caso especial.
 * Cada función detalla sus posibles valores de retorno negativos.
 * Más allá de los errores que devuelvan estas funciónes, probablemente los wrappers retornen -5 en el caso de falta de permisos
*/
int64_t readFifo(uint64_t fileId, void *buffer, uint32_t size); // Retornos < 0: -1 para EOF, -2 para archivo no encontrado 
int64_t writeFifo(uint64_t fileId, void *buffer, uint32_t size); // Retornos < 0: -1 para archivo cerrado para escritura, -2 para archivo no encontrado

int64_t readRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset);             // Retornos < 0: -1 para offset fuera de rango, -2 para archivo no encontrado
int64_t writeRaw(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset);            // Retornos < 0: -1 para offset fuera de rango, -2 para archivo no encontrado
int64_t writeRawWithRealloc(uint64_t fileId, void *buffer, uint32_t size, uint32_t offset); // Retornos < 0: -1 para error de reallocación, -2 para archivo no encontrado

// Devuelve un archivo con file id 0 en el caso de que no se encuentre el archivo
File getFileById(uint64_t fileId);
uint64_t * listFiles(); // devuelve los IDs de todos los archivos, ordenados por path (es más un tree que un ls)

// Solo el owner de escritura o algo corriendo en modo kernel pueden cambiar los permisos de un archivo
//      -> Para indicar que se llama como kernel, pasar un pid = 0; eso bypasea el chequeo de permisos
// Retorna 0 si se pudo cambiar, -1 si no se pudo cambiar 
int setFilePermissions(uint64_t fileId, Pid pid, FilePermissions permissions);

// Cambia la ruta del archivo, devuelve 0 si se pudo cambiar, -1 si no se pudo cambiar
int setFilePath(uint64_t fileId, const char *newPath); 

#endif