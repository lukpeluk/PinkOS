#include <audioLib.h>
#include <stdint.h>
#include <stdpink.h>

void resume_main() {
    enableBackgroundAudio();  // requiere acceso a controlar el sonido de fondo
    resume_audio();
}

void pause_main() {
    enableBackgroundAudio();  // requiere acceso a controlar el sonido de fondo
    pause_audio();            // al llamar cualquier comando con permisos de audio el sonido se pausa automáticamente, así que técnicamente no hace falta
}
