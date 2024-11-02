#ifndef STDIN_H
#define STDIN_H

// función de la shell, quien es quien gestiona la entrada estándar
// devuelve 0 si no hay nada en el buffer
unsigned char get_char_from_stdin();
void clear_stdin();

#endif