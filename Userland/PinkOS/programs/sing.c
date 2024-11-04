#include <audioLib.h>
#include <stdint.h>
#include <stdpink.h>
#include <songs.h>


// args should be a number representing the choosen song
void sing_main(char *args){
    int i = 0;
    int background = 0;
    if(args[0] == '-' && args[1] == 'b' && (args[2] == ' ' || args[2] == '\0')){
        background = 1;
        i = 3;
    }

    if(args[i] == '\0' || (args[i] < '0' || args[i] > '9') || args[i+1] != '\0'){
        print("Usage: sing [-b] <song number>\nWill play the Pink Panther theme while you think about it.\n\n");
        args[i] = '0';
    }
    print("You can use space to pause/resume :)\n\n");

    switch (args[i])
    {
        case '0':
            play_audio(pinkPanther, background, 120);
            break;
        case '1':
            play_audio(superMarioBros, background, 180);
            break;
        case '2':
            play_audio(furElise, background, 60);
            break;
        default:
            break;
    }

    if(background){
        enableBackgroundAudio();
        return;
    }

    // Wait for the song to finish
    int paused = 0;
    while(is_audio_playing() || paused){
        if(getChar() == ' '){
            if(paused){
                print("resuming\n");
                resume_audio();
            } else {
                print("pausing\n");
                pause_audio();
            }
            paused = !paused;
        }
    }
}