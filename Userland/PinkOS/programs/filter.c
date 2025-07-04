#include <libs/stdpink.h>

#define BUFFER_SIZE 256

void filter_main(char *args) {
    char buffer[BUFFER_SIZE];
    int bytes_read;

    while((bytes_read = readStdin(buffer, BUFFER_SIZE)) >= 0) {
        for (int i = 0; i < bytes_read; i++) {
            char c = buffer[i];
            if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
                c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U') {
                printf("%c", c);
            }
        }
    }
}