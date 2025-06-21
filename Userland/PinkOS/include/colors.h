#ifndef COLORS_H
#define COLORS_H

#include <stdint.h>

typedef struct Colors {
    // UI colors
    uint32_t primary;
    uint32_t secondary;
    uint32_t highlight;
    uint32_t background;
    uint32_t status_bar_text;
    uint32_t status_bar_background;
    uint32_t prompt;

    // Text colors (for stdin/out)
    uint32_t text;
    uint32_t text_secondary;
    uint32_t highlighted_text;
    uint32_t highlighted_text_background;
    uint32_t error;
    uint32_t success;
    uint32_t warning;
    uint32_t info;
} Colors;


extern Colors* ColorSchema; // Representa el esquema de colores actual, se inicializa en PinkOS.c, capaz hay que buscarle otro lugar


#endif