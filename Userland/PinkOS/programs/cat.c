#include <libs/stdpink.h>

#define BUFFER_SIZE 256

void cat_main(char *args)
{
    char buffer[BUFFER_SIZE];
    char line_buffer[BUFFER_SIZE];
    int bytes_read;
    int line_pos = 0;
    
    // Leer de stdin hasta que llegue EOF (-1)
    while ((bytes_read = readStdin(buffer, BUFFER_SIZE)) >= 0) {
        for (int i = 0; i < bytes_read; i++) {
            line_buffer[line_pos] = buffer[i];
            line_pos++;
            
            if (buffer[i] == '\n' || line_pos >= BUFFER_SIZE - 1) {
                line_buffer[line_pos] = '\0';
                print(line_buffer); //imprimo cuando encuentro un '/n' o se llena el buffer
                line_pos = 0;
            }
        }
    }
    
    if (line_pos > 0) {
        line_buffer[line_pos] = '\0';
        print(line_buffer);
    }
}
