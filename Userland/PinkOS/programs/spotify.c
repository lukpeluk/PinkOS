#include <libs/audioLib.h>
#include <stdint.h>
#include <libs/stdpink.h>
#include <songs.h>


char tolower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}

int strcasecmp(const char* a, const char* b) {
    while (*a && *b && tolower(*a) == tolower(*b)) {
        a++;
        b++;
    }
    return tolower(*a) - tolower(*b);
}

int calculate_similarity(const char* a, const char* b) {
    int matches = 0;
    int length = 0;
    while (*a && *b) {
        if (*a == *b) {
            matches++;
        }
        a++;
        b++;
        length++;
    }
    while (*b) {
        b++;
        length++;
    }
    return (int) (100 * matches / length);
}


// args should be a number representing the choosen song
void spotify_main(char *args){
    int i = 0;
    int background = 0;
    int song_found = 0;
    if(args[0] == '-' && args[1] == 'b' && (args[2] == ' ' || args[2] == '\0')){
        background = 1;
        i = 3;
    }

    if(args[i] == '\0'){
        print((char *)"Usage: spotify [-b] <song name>\nWill play the Pink Panther theme while you think about it.\n\n");
        args = (char *)"pink panther";
        play_audio(songs[0].notes, background, songs[0].tempo);
        song_found = 1;

    }

    for (int j = 0; j < num_songs; j++) {
        if (calculate_similarity(args + i, songs[j].name) > 50) {
            play_audio(songs[j].notes, background, songs[j].tempo);
            song_found = 1;
            break;
        }
    }

    if (!song_found)
    {
        print((char *)"Song not found. :(\n");
        return;
        // print("Song not found. Playing Pink Panther theme instead.\n");
        // play_audio(songs[0].notes, background, songs[0].tempo);
    }
    print((char *)"You can use space to pause/resume :)\n\n");

    if(background){
        enableBackgroundAudio();
        return;
    }

    // Wait for the song to finish
    int paused = 0;
    while(is_audio_playing() || paused){
        if(getChar() == ' '){
            if(paused){
                print((char *)"resuming\n");
                resume_audio();
            } else {
                print((char *)"pausing\n");
                pause_audio();
            }
            paused = !paused;
        }
    }
}