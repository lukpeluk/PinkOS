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

static const Colors PinkOSMockupColors = {
    .primary = 0x00f0489f,
    .secondary = 0x00df8090,
    .highlight = 0x00f5d8d3,
    .background = 0x00fed6d6,
    .status_bar_text = 0x00fed6d6,
    .status_bar_background = 0x00fd67ae,
    .text = 0x00fd67ae,
    .text_secondary = 0x00ef90a0,
    .highlighted_text = 0x00fd67ae,
    .highlighted_text_background = 0x00e5d8d3,
    .prompt = 0x00f0489f,
    .error = 0x00ff2c2c,
    .success = 0x0073ba52,
    .warning = 0x00ff4f00,
    .info = 0x00b5a3c5
};


static const Colors PinkOSColors = {
    .primary = 0x00f0489f,
    .secondary = 0x00df8090,
    .highlight = 0x00f5d8d3,
    .background = 0x00000000,
    .status_bar_text = 0x00f5d8d3,
    .status_bar_background = 0x00cf7085,
    .prompt = 0x00f5d8d3,

    .text = 0x00df8090,
    .text_secondary = 0x00df8090,
    .highlighted_text_background = 0x00e5d8d3,
    .error = 0x00c92836,    
    .success = 0x0073ba52,
    .warning = 0x00c2664f,
    .info = 0x00b5a3c5
};




#endif