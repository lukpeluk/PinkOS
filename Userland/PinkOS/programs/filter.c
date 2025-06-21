#include <libs/stdpink.h>

#define BUFFER_SIZE 256

void filter_main(char *args) {
    char buffer[BUFFER_SIZE];
    char line_buffer[BUFFER_SIZE];
    int bytes_read;
    int line_pos = 0;

    while((bytes_read = readStdin(buffer, sizeof(buffer))) >= 0) {
        for (int i = 0; i < bytes_read; i++) {
            char c = buffer[i];
            if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' &&
                c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U') {
                line_buffer[line_pos] = c;
                line_pos++;
            }
            if (c == '\n' || line_pos >= BUFFER_SIZE - 1) {
                line_buffer[line_pos] = '\0';
                print(line_buffer);
                line_pos = 0;
            }
        }
    }

    if (line_pos > 0) {
        line_buffer[line_pos] = '\0';
        print(line_buffer);
    }
}