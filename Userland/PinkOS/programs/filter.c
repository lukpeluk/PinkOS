#include <libs/stdpink.h>

void filter_main(char *args) {
    char buffer[1024];
    int bytes_read;
    while((bytes_read = readStdin(buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            char c = buffer[i];
            if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
                c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U') {
                printf((char *)"%c", c);
            }
        }
    }
}