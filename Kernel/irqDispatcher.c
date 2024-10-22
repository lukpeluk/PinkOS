#include <time.h>
#include <stdint.h>
#include <naiveConsole.h>
#include <keyboardInterrupt.h>

static void int_20();

void irqDispatcher(uint64_t irq) {
	switch (irq) {
		// acá se llaman a las diferentes funciones de interrupción
		case 0:
			// int_20();
			break;

		case 1:
			int_21();
			break;
	}
	return;
}

void int_20() {
	timer_handler();
	int a = ticks_elapsed();
	ncPrintDec(a);
}
