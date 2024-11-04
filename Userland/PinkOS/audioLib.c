#include <audioLib.h>
#include <stdint.h>
#include <syscallCodes.h>
#include <stdpink.h>

extern void syscall(uint64_t syscall, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

void play_audio(Note** notes, uint8_t loop, uint64_t tempo){
    syscall(PLAY_AUDIO_SYSCALL, (uint64_t)notes, loop, tempo, 0, 0);
}

void stop_audio(){
    syscall(STOP_AUDIO_SYSCALL, 0, 0, 0, 0, 0);
}

void pause_audio(){
    syscall(PAUSE_AUDIO_SYSCALL, 0, 0, 0, 0, 0);
}

void resume_audio(){
    syscall(RESUME_AUDIO_SYSCALL, 0, 0, 0, 0, 0);
}

AudioState get_audio_state(){
    AudioState audio_state;
    syscall(GET_AUDIO_STATE_SYSCALL, (uint64_t)&audio_state, 0, 0, 0, 0);
    return audio_state;
}

void load_audio_state(AudioState state){
    syscall(LOAD_AUDIO_STATE_SYSCALL, (uint64_t)&state, 0, 0, 0, 0);
}

int is_audio_playing(){
    int is_playing;
    syscall(IS_AUDIO_PLAYING_SYSCALL, &is_playing, 0, 0, 0, 0);
    return is_playing;
}


