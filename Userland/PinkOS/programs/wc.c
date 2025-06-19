#include <libs/stdpink.h>

void wc_main(char *args) {
    char buffer[1024];
    int line_count = 0;

    int bytes_read;
    while((bytes_read = readStdin(buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                line_count++;
            }
        }
    }

    printf((char *)"Number of lines: %d\n", line_count);
}

