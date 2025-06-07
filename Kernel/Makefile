include Makefile.inc

KERNEL=kernel.bin
SOURCES=$(wildcard src/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
DRIVER_SOURCES=$(wildcard src/drivers/*.c)
SCHEDULER_SOURCES=$(wildcard src/scheduling/*.c)
EVENT_HANDLING_SOURCES=$(wildcard src/eventHandling/*.c)
INTERRUPT_SOURCES=$(wildcard src/interrupts/*.c)
EXCEPTION_SOURCES=$(wildcard src/exceptions/*.c)
SYSCALLS_SOURCES=$(wildcard src/syscalls/*.c)

OBJECTS=$(SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
DRIVER_OBJECTS=$(DRIVER_SOURCES:.c=.o)
EVENT_HANDLING_OBJECTS=$(EVENT_HANDLING_SOURCES:.c=.o)
INTERRUPT_OBJECTS=$(INTERRUPT_SOURCES:.c=.o)
EXCEPTION_OBJECTS=$(EXCEPTION_SOURCES:.c=.o)
SYSCALL_OBJECTS=$(SYSCALLS_SOURCES:.c=.o)
SCHEDULER_OBJECTS=$(SCHEDULER_SOURCES:.c=.o)

LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(DRIVER_OBJECTS) $(SCHEDULER_OBJECTS) $(EVENT_HANDLING_OBJECTS) $(INTERRUPT_OBJECTS) $(EXCEPTION_OBJECTS) $(SYSCALL_OBJECTS)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(DRIVER_OBJECTS) $(SCHEDULER_OBJECTS) $(EVENT_HANDLING_OBJECTS) $(INTERRUPT_OBJECTS) $(EXCEPTION_OBJECTS) $(SYSCALL_OBJECTS)

%.o: %.c
	$(GCC) $(GCCFLAGS) -I./include -c $< -o $@

%.o : %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(LOADEROBJECT):
	$(ASM) $(ASMFLAGS) $(LOADERSRC) -o $(LOADEROBJECT)

clean:
	find . -type f -name '*.o' -delete
	find . -type f -name '*.bin' -delete

.PHONY: all clean
