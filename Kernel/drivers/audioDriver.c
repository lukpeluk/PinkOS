
#include <stdint.h>

extern void play_sound(uint32_t nFrequence);
extern void stop_sound();

void beep(uint64_t freq, uint64_t milis) {
    if(freq == 0) return;
    play_sound(freq);
    sleep(milis);
    stop_sound();
}