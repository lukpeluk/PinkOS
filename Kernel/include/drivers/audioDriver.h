#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <stdint.h>

typedef struct Note{
    uint64_t freq;      // 0 for a rest
    int duration;       // represents the duration of the note (1 for whole note, 2 for half note, 4 for quarter note, etc)
                        // negative values represent dotted notes (e.g. -4 for a dotted quarter note)
                        // the duration is relative to the tempo of the song
    uint8_t ligated;     // 1 if the note is ligated with the next one, 0 otherwise
} Note;

typedef struct AudioState {
    uint8_t playing;
    uint8_t loop;
    uint64_t tempo;
    Note** notes;                  // null terminated
    uint64_t current_note;
    uint64_t current_note_start;  // in milliseconds
    uint64_t current_note_duration;
} AudioState;


// main loop (intended to be called every timer tick, to manage the audio stream)
void audioLoop();

// audio controlls
void play_audio(Note** notes, uint8_t loop, uint64_t tempo);
void stop_audio();
void pause_audio();
void resume_audio();

int is_audio_playing();

int get_milliseconds_delayed();  // useful for debugging, tells you how many milliseconds the audio was delayed

uint64_t get_tempo();
void set_tempo(uint64_t tempo);
void inc_tempo(int amount);     // negative values to decrease tempo


// audio state 
AudioState get_audio_state();
void load_audio_state(AudioState state);


#endif