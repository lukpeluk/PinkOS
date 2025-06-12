#include <time.h>
#include <stdint.h>
#include <naiveConsole.h>
#include <interrupts/keyboardInterrupt.h>
#include <interrupts/serialInterrupt.h>
#include <interrupts/rtcInterrupt.h>
#include <drivers/mouseDriver.h>

#include <drivers/videoDriver.h> // maybe for debugging

void timer_handler();

static void int_20();

void irqDispatcher(uint64_t irq) {
    switch (irq) {
        // acá se llaman a las diferentes funciones de interrupción
        case 0:
            int_20();
            break;

        case 1:
            int_21();
            break;
        case 8:
            int_28();
            break;
        case 3:
            int_23();
            break;

        case 12:
            log_to_serial("IRQ 12 (mouse) received - calling mouse_handler()");
            mouse_handler();
            break;

        case 80:
            // las syscals no se gestionan acá, tienen un handler distinto en interrupts.asm
            // esto es para poder pasar los argumentos directo a syscallHandler
            // (sin que se sobreescriba rdi al llamar a irqDispatcher)
            break;
    }
    return;
}

void int_20() {
    timer_handler();
}



