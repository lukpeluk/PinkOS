#ifndef THEMES_H
#define THEMES_H
#include <colors.h>

#include <stdint.h>


static Colors PinkOSMockupColors = {
    .primary = 0x00f0489f,
    .secondary = 0x00df8090,
    .highlight = 0x00f5d8d3,
    .background = 0x00fed6d6,
    .status_bar_text = 0x00fed6d6,
    .status_bar_background = 0x00fd67ae,
    .prompt = 0x00f0489f,

    .text = 0x00fd67ae,
    .text_secondary = 0x00ef90a0,
    .highlighted_text = 0x00fd67ae,
    .highlighted_text_background = 0x00443d4a,
    .error = 0x00ff2c2c,
    .success = 0x0073ba52,
    .warning = 0x00ff4f00,
    .info = 0x00b5a3c5
};


static Colors PinkOSColors = {
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