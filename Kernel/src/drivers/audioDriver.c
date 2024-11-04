
#include <stdint.h>
#include <drivers/audioDriver.h>
#include <drivers/videoDriver.h>

#define CURRENT_NOTE() audioState.notes[audioState.current_note]
#define NOTE_STILL_PLAYING() (audioState.current_note_duration + audioState.current_note_start > milliseconds_elapsed())
#define NOTE_PERCENTAGE_PLAYED() ((milliseconds_elapsed() - audioState.current_note_start) * 100 / getNoteDuration(CURRENT_NOTE()))
#define WHOLE_NOTE() ((60000 * 4) / audioState.tempo)

extern void play_sound(uint32_t nFrequence);
extern void stop_sound();

static AudioState audioState = {0};

uint64_t getNoteDuration(Note * note) {
    int duration = note->duration;
    if(duration < 0) {
        duration = -duration;
        return (WHOLE_NOTE() / duration) * 1.5;
    } else {
        return WHOLE_NOTE() / duration;
    }
}

static current_note;

// main loop (intended to be called every timer tick, to manage the audio stream)

// for limitations with this architecture, the duration of a note is limited by the frequency at which you call this function (e.g. calling it every timer tick means no note can be < 55 ms)
// if you have notes with a duration < 55 ms, those notes will be played for 55 ms, and if that happens a lot the song will slowly get out of sync
// this is not something you can notice really, its just a technical limitation I thought was worth mentioning
void audioLoop() {
    if(!audioState.playing) return;

    if(NOTE_STILL_PLAYING()){
        if(!CURRENT_NOTE()->ligated && NOTE_PERCENTAGE_PLAYED() > 90) // this adds a small pause between notes that are not ligated
             stop_sound();
        return;
    }

    // if the note is still playing, but the function was called after the note should have ended, we need to skip part of the next note to be in sync
    uint64_t intended_stop_time = audioState.current_note_start + audioState.current_note_duration;
    int time_difference = milliseconds_elapsed() - intended_stop_time;

    audioState.current_note++;
    if(CURRENT_NOTE() == 0){
        if(audioState.loop){
            audioState.current_note = 0;
        } else {
            audioState = (AudioState){0};
            stop_sound();
            return;
        }
    }

    audioState.current_note_start = milliseconds_elapsed();
    int duration = getNoteDuration(CURRENT_NOTE()) - time_difference;
    audioState.current_note_duration = duration > 0 ? duration : 0;

    play_sound_wrapper(CURRENT_NOTE()->freq);
}

// functions to expose to the user, to control the audio stream
// ============================================================

void play_audio(Note** notes, uint8_t loop, uint64_t tempo) {
    if(notes[0] == 0) return;
    audioState.playing = 1;
    audioState.tempo = tempo;
    audioState.loop = loop;
    audioState.notes = notes;
    audioState.current_note = 0;
    audioState.current_note_start = milliseconds_elapsed();
    audioState.current_note_duration = getNoteDuration(notes[0]);

    play_sound_wrapper(notes[0]->freq);
}

void stop_audio() {
    audioState = (AudioState){0};
    stop_sound();
}

void pause_audio() {
    if(!audioState.playing) return;

    audioState.playing = 0;

    audioState.current_note_duration = audioState.current_note_duration - NOTE_PERCENTAGE_PLAYED();
    stop_sound();
}

void resume_audio() {
    if(audioState.playing || audioState.notes == 0) return;

    audioState.playing = 1;

    audioState.current_note_start = milliseconds_elapsed();
    play_sound_wrapper(CURRENT_NOTE()->freq);
}

// usefull for switching between audio streams without losing the previous state
// intended usage:
// ================
// pause_audio();
// AudioState prev_state = get_audio_state();
// play_audio(new_notes, loop);
// load_audio_state(prev_state);
// resume_audio();
AudioState get_audio_state() {
    return audioState;
}

void load_audio_state(AudioState state) {
    audioState = state;
}

int is_audio_playing() {
    return audioState.playing;
}

// ============================================


void play_sound_wrapper(uint64_t freq) {
    if(freq == 0)
        stop_sound();
    else
        play_sound(freq);
}

void beep(uint64_t freq, uint64_t milis) {
    if(freq == 0) return;
    play_sound(freq);
    sleep(milis);
    stop_sound();
}
