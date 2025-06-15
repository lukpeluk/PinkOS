#include <libs/stdpink.h>

#include <libs/graphicsLib.h>
#include <pictures.h>
#include <libs/audioLib.h>
#include <songs.h>
#include <keyboard.h>

#include <syscalls/syscallCodes.h>
#include <environmentApiEndpoints.h>
#include <handlerIds.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3);

void test(char event_type, int hold_times, char ascii, char scan_code){
    print((char *)"System hijacked! >:)\n");
}

void demo_main(char * args){
    // // System reserved tasks
    // syscall(SET_HANDLER_SYSCALL, KEY_HANDLER, (uint64_t)test, 0);

    // // CLI
    // clear();
    // print((char *)"Welcome to PinkOS Demo from the user environment API!");


    // // GUI
    // syscall(CLEAR_SCREEN_SYSCALL, 0, 0, 0);
    // syscall(DRAW_STRING_SYSCALL, (uint64_t) "Welcome to PinkOS demo directly from a syscall!", 0xffffff, 0x000000);


    // // Audio
    // play_audio(songs[0].notes, 0, songs[0].tempo);
    // while(is_audio_playing());
}
