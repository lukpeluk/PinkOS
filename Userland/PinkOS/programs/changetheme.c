#include <programs.h>
#include <stdint.h>
#include <stdpink.h>
#include <colors.h>


void change_theme_main(unsigned char *args){
    if (ColorSchema == &PinkOSColors) {
        ColorSchema = &PinkOSMockupColors;
    } else {
        ColorSchema = &PinkOSColors;
    }
    clear_main(args);
}