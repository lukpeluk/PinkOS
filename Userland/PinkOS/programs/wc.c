#include <libs/stdpink.h>
#include <libs/serialLib.h>

void wc_main(char *args) {
    char buffer[1024];
    int line_count = 1;
    int word_count = 0;
    int char_count = 0;
    char last_char = 0;

    int bytes_read;
    while((bytes_read = readStdin(buffer, sizeof(buffer))) >= 0) {
        for (int i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n') {
                line_count++;
            }
            if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\t') {
                if (i > 0 && (buffer[i-1] != ' ' && buffer[i-1] != '\n' && buffer[i-1] != '\t')) {
                    word_count++;
                }
            }
            if (buffer[i] != '\n' && buffer[i] != '\t') {
                char_count++;
            }
            last_char = buffer[i];
            // printf((char *)"%c", buffer[i]); // Print each character to stdout
        }
    }
    if (char_count > 0 && (last_char != ' ' && last_char != '\n' && last_char != '\t')) {
        word_count++;
    }
    printf((char *)"Number of lines: %d\n", line_count);
    printf((char *)"Number of words: %d\n", word_count);
    printf((char *)"Number of characters: %d\n", char_count);
}

