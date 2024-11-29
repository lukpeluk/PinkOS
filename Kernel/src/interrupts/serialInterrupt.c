#include <interrupts/serialInterrupt.h>
#include <drivers/serialDriver.h>

void int_23() {
    unsigned char c = read_serial();
    // write_serial(c); un loro

    process_serial(c);
}