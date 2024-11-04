#include <stdint.h>
#include <keyboard.h>
#include <programs.h>
#include <permissions.h>
#include <graphicsLib.h>
#include <stdin.h>
#include <syscallCodes.h>
#include <stdpink.h>
#include <environmentApiEndpoints.h>
#include <math.h>
#include <audioLib.h>
#include <pictures.h>
#include <ascii.h>

extern uint64_t syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);


int str_to_int(char *str) {
    int result = 0;
    int i = 0;
    while (str[i] != '\0' && str[i] >= '0' && str[i] <= '9') {
        result = result * 10 + str[i] - '0';
        i++;
    }
    return result;
}


Note * testChromaticScaleLigated[] = {
    &(Note) {262, 1, 1},
    &(Note) {277, 1, 1},
    &(Note) {294, 1, 1},
    &(Note) {311, 1, 1},
    &(Note) {330, 1, 1},
    &(Note) {349, 1, 1},
    &(Note) {370, 1, 1},
    &(Note) {392, 1, 1},
    &(Note) {415, 1, 1},
    &(Note) {440, 1, 1},
    &(Note) {466, 1, 1},
    &(Note) {494, 1, 1},
    &(Note) {523, 1, 0},
    0,
};

void mona_lisa_main(char *args) {
    if(args[0] == '\0'){
		print("Usage: mona_lisa <scale (range 1-15)>\n");
        return;
    }

	play_audio(testChromaticScaleLigated, 1, 120);

    // get scale from arguments
    int desired_scale = 1;
    if (args[0] != '\0' && args[0] >= '0' && args[0] <= '9') {
        desired_scale = str_to_int(args);
    }

    if(desired_scale < 1 || desired_scale > 15){
		print("Usage: mona_lisa <scale (range 1-15)>\n");
        return;
    }

    Point position = {0};

    int screen_width = getScreenWidth();
    int screen_height = getScreenHeight();

    int scale = 1;
    while (1) {
         // Center the image
        position.x = (screen_width - MONA_LISA_WIDTH * scale) / 2;
        position.y = (screen_height - MONA_LISA_HEIGHT * scale) / 2;

        // Draw the Mona Lisa image
        drawBitmap(mona_lisa, MONA_LISA_WIDTH, MONA_LISA_HEIGHT, position, scale);

        if(scale == desired_scale){
			// scale = 1;
            break;
        }

        scale++;
        syscall(SLEEP_SYSCALL, 100, 0, 0, 0, 0);
    }

    // Wait for key
    char c = get_char_from_stdin();
    while(1){
        // if (c != 0) {
        //     break;
        // }
        // c = get_char_from_stdin();
    }
}