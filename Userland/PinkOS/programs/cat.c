#include <libs/stdpink.h>

void cat_main(char *args)
{
    char buffer[1025];  // Buffer para leer chunks de datos + null terminator
    int bytes_read;
    
    // Leer de stdin hasta que llegue EOF (-1)
    while ((bytes_read = readStdin(buffer, sizeof(buffer) - 1)) > 0) {
        // Agregar null terminator para poder usar print
        buffer[bytes_read] = '\0';
        
        // Usar print en lugar de writeStdout
        print(buffer);
    }
    
    // Si bytes_read es -1, significa EOF
    // Si bytes_read es -2, significa que no hay stdin asignado
    if (bytes_read == -2) {
        printf("cat: no stdin assigned\n");
    }
}
