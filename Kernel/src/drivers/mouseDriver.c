
#include <drivers/keyboardDriver.h>
#include <drivers/mouseDriver.h>
#include <drivers/serialDriver.h>


#define MOUSE_IRQ 12
#define MOUSE_DATA_PORT 0x60
#define MOUSE_CMD_PORT  0x64

#define SCROLL_UP 0x01
#define SCROLL_DOWN 0xFF // -1 interpretado como 0xFF

#define MOUSE_PACKET_SIZE 3  // Cambiar a 3 porque no tenemos IntelliMouse funcionando
static char mouse_bytes[MOUSE_PACKET_SIZE];


extern void simulateKeyPress(unsigned char scan_code); // Deberías tener esto en tu teclado

static int mouse_cycle = 0;
// static char mouse_bytes[4];


static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0"
                      : "=a"(ret)
                      : "Nd"(port));
    return ret;
}

static void mouse_wait_input(void) {
    for (int i = 0; i < 1000000; i++) {  // Aumentado de 100000 a 1000000
        if ((inb(MOUSE_CMD_PORT) & 2) == 0)
            return;
    }
    // log_to_serial("E: Timeout esperando input");
}

static void mouse_wait_output(void) {
    for (int i = 0; i < 1000000; i++) {  // Aumentado de 100000 a 1000000
        if (inb(MOUSE_CMD_PORT) & 1)
            return;
    }
    // log_to_serial("E: Timeout esperando output");
}

static void mouse_write(uint8_t data) {
	mouse_wait_input();
	outb(MOUSE_CMD_PORT, 0xD4); // Señala que se va a mandar al mouse
	mouse_wait_input();
	outb(MOUSE_DATA_PORT, data);
}

static uint8_t mouse_read(void) {
	mouse_wait_output();
	return inb(MOUSE_DATA_PORT);
}


void init_mouse() {
	// log_to_serial("I: Inicializando mouse");

	// Habilitar segundo puerto (mouse)
	outb(MOUSE_CMD_PORT, 0xA8);

	// Leer config byte, setear bit 1 (habilita IRQ12)
	outb(MOUSE_CMD_PORT, 0x20);
	uint8_t status = inb(MOUSE_DATA_PORT);
	status |= 2;
	outb(MOUSE_CMD_PORT, 0x60);
	outb(MOUSE_DATA_PORT, status);

	// Inicializar mouse (modo básico)
	mouse_write(0xF6); mouse_read(); // Set defaults
	mouse_write(0xF4); mouse_read(); // Enable data reporting

	// Activar modo IntelliMouse para soporte de scroll
	// log_to_serial("Activating IntelliMouse mode...");
	mouse_write(0xF3); mouse_read(); mouse_write(200); mouse_read();
	mouse_write(0xF3); mouse_read(); mouse_write(100); mouse_read();
	mouse_write(0xF3); mouse_read(); mouse_write(80);  mouse_read();

	// Verificar si se activó correctamente
	mouse_write(0xF2); // Get device ID
	uint8_t id = mouse_read(); // Debería devolver 0x03 para IntelliMouse
	// log_hex("Mouse ID: ", id);
	
	if (id == 0x03) {
		// log_to_serial("IntelliMouse mode activated successfully (ID=0x03)");
	} else if (id == 0x00) {
		// log_to_serial("WARNING: Standard mouse mode (ID=0x00), no scroll wheel");
	} else {
		// log_hex("WARNING: Unknown mouse ID: ", id);
	}

	// log_to_serial("I: Mouse initialized successfully");
}


void simulateKeyPress(unsigned char scan_code) {
	KeyboardEvent e = processScancode(scan_code);

	if(e.event_type != 0) {
		handleKeyEvent(e);
	}
}


void mouse_handler() {
	static int interrupt_count = 0;
	interrupt_count++;
	
	// log_decimal("Mouse interrupt #", interrupt_count);
	
	uint8_t byte = inb(MOUSE_DATA_PORT);
	mouse_bytes[mouse_cycle] = byte;
	
	// log_hex("Received byte", mouse_cycle);
	// log_hex(": ", byte);
	
	mouse_cycle++;

	if (mouse_cycle < MOUSE_PACKET_SIZE)
		return;

	mouse_cycle = 0;

	// Debug: imprimir todos los bytes del paquete
	// log_to_serial("Mouse packet complete:");
	for (int i = 0; i < MOUSE_PACKET_SIZE; i++) {
		// log_hex("  Byte", i);
		// log_hex(": ", mouse_bytes[i]);
	}

	// En modo estándar (3 bytes), el scroll se detecta por cambios en Y
	// Byte 0: flags, Byte 1: X movement, Byte 2: Y movement
	char x_movement = mouse_bytes[1];
	char y_movement = mouse_bytes[2];
	
	// log_hex("X movement: ", x_movement);
	// log_hex("Y movement: ", y_movement);


	// Detectar scroll basado en patrones observados:
	// Scroll hacia arriba: X=0x01, Y=0x08
	// Scroll hacia abajo: X=0xFF, Y=0x08
	if (x_movement == 0x01 && y_movement == 0x08) {
		// log_to_serial("SCROLL UP detected");
		simulateKeyPress(0xE0);
		simulateKeyPress(0x49);  // Page Up press
		simulateKeyPress(0xE0);
		simulateKeyPress(0xC9);  // Page Up release
	} else if (x_movement == 0xFF && y_movement == 0x08) {
		// log_to_serial("SCROLL DOWN detected");
		simulateKeyPress(0xE0);
		simulateKeyPress(0x51);  // Page Down press
		simulateKeyPress(0xE0);
		simulateKeyPress(0xD1);  // Page Down release
	}
}