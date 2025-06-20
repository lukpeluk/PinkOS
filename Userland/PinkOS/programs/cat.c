#include <libs/stdpink.h>

#define BUFFER_SIZE 1025

void cat_main(char *args)
{
    char buffer[BUFFER_SIZE];  // Buffer para leer chunks de datos + null terminator
    int bytes_read;
    char c;
    int i = 0;
    
    // Leer de stdin hasta que llegue EOF (-1)
    while ((bytes_read = readStdin(buffer, 1)) >= 0) {
        do
        {
            bytes_read = readStdin(&c, 1);
            if (bytes_read > 0 && i < BUFFER_SIZE - 1) {
                buffer[i] = c;
                i++;
            }
        } while (bytes_read >= 0 && c != '\n');
        print(buffer);
        buffer[0] = '\0'; 
        i = 0; 
    }

    if (bytes_read == -2) {
        print("cat: no stdin assigned\n");
    } else if (bytes_read == -1) {
        print("cat: EOF reached\n");
    }
    else {
        print("cat: error reading stdin\n");
    }
}
