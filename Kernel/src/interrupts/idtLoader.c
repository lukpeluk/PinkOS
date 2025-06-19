#include <stdint.h>
#include <defs.h>
#include <interrupts/idtLoader.h>
#include <interrupts/interrupts.h>

#pragma pack(push)		/* Push de la alineación actual */
#pragma pack (1) 		/* Alinear las siguiente estructuras a 1 byte */

/* Descriptor de interrupcion */
typedef struct {
  uint16_t offset_l, selector;
  uint8_t cero, access;
  uint16_t offset_m;
  uint32_t offset_h, other_cero;
} DESCR_INT;

#pragma pack(pop)		/* Reestablece la alinceación actual */



DESCR_INT * idt = (DESCR_INT *) 0;	// IDT de 255 entradas

void load_idt() {

  setup_IDT_entry (0x00, (uint64_t)&_exception0Handler);
  setup_IDT_entry (0x01, (uint64_t)&_exception1Handler);
  setup_IDT_entry (0x02, (uint64_t)&_exception2Handler);
  setup_IDT_entry (0x03, (uint64_t)&_exception3Handler);
  setup_IDT_entry (0x04, (uint64_t)&_exception4Handler);
  setup_IDT_entry (0x05, (uint64_t)&_exception5Handler);
  setup_IDT_entry (0x06, (uint64_t)&_exception6Handler);
  setup_IDT_entry (0x07, (uint64_t)&_exception7Handler);
  setup_IDT_entry (0x08, (uint64_t)&_exception8Handler);
  setup_IDT_entry (0x09, (uint64_t)&_exception9Handler);
  setup_IDT_entry (0x0A, (uint64_t)&_exception10Handler);
  setup_IDT_entry (0x0B, (uint64_t)&_exception11Handler);
  setup_IDT_entry (0x0C, (uint64_t)&_exception12Handler);
  setup_IDT_entry (0x0D, (uint64_t)&_exception13Handler);
  setup_IDT_entry (0x0E, (uint64_t)&_exception14Handler);
  setup_IDT_entry (0x0F, (uint64_t)&_exception15Handler);
  setup_IDT_entry (0x10, (uint64_t)&_exception16Handler);
  setup_IDT_entry (0x11, (uint64_t)&_exception17Handler);
  setup_IDT_entry (0x12, (uint64_t)&_exception18Handler);
  setup_IDT_entry (0x13, (uint64_t)&_exception19Handler);
  setup_IDT_entry (0x14, (uint64_t)&_exception20Handler);
  setup_IDT_entry (0x15, (uint64_t)&_exception21Handler);
  setup_IDT_entry (0x16, (uint64_t)&_exception22Handler);
  setup_IDT_entry (0x17, (uint64_t)&_exception23Handler);
  setup_IDT_entry (0x18, (uint64_t)&_exception24Handler);
  setup_IDT_entry (0x19, (uint64_t)&_exception25Handler);
  setup_IDT_entry (0x1A, (uint64_t)&_exception26Handler);
  setup_IDT_entry (0x1B, (uint64_t)&_exception27Handler);
  setup_IDT_entry (0x1C, (uint64_t)&_exception28Handler);
  setup_IDT_entry (0x1D, (uint64_t)&_exception29Handler);
  setup_IDT_entry (0x1E, (uint64_t)&_exception30Handler);
  setup_IDT_entry (0x1F, (uint64_t)&_exception31Handler);
	setup_IDT_entry (0x20, (uint64_t)&_irq00Handler);
	setup_IDT_entry (0x21, (uint64_t)&_irq01Handler);
  setup_IDT_entry (0x28, (uint64_t)&_irq08Handler);
	setup_IDT_entry (0x80, (uint64_t)&_irq80Handler);
  setup_IDT_entry(0x2C, (uint64_t)&_irq0CHandler);

  setup_IDT_entry(0x23, (uint64_t)&_irq03Handler); // IRQ3 is for COM1, para el serial
  setup_IDT_entry(0x24, (uint64_t)&_irq03Handler); // IRQ3 is for COM1, para el serial


	
	picMasterMask(0xE0); 
	// picSlaveMask(0xFE);
  picSlaveMask(0xEE); 

	// _sti();
}

static void setup_IDT_entry (int index, uint64_t offset) {
  idt[index].selector = 0x08;
  idt[index].offset_l = offset & 0xFFFF;
  idt[index].offset_m = (offset >> 16) & 0xFFFF;
  idt[index].offset_h = (offset >> 32) & 0xFFFFFFFF;
  idt[index].access = ACS_INT;
  idt[index].cero = 0;
  idt[index].other_cero = (uint64_t) 0;
}



