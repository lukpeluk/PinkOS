include Makefile.inc

KERNEL=kernel.bin
SOURCES=$(wildcard src/*.c)
SUB_SOURCES=$(wildcard src/*/*.c)
SOURCES_ASM=$(wildcard asm/*.asm)

OBJECTS=$(SOURCES:.c=.o)
SUB_OBJECTS=$(SUB_SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)

LOADERSRC=loader.asm

LOADEROBJECT=$(LOADERSRC:.asm=.o)
STATICLIBS=

all: $(KERNEL)

# $(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(DRIVER_OBJECTS) $(SCHEDULER_OBJECTS) $(EVENT_HANDLING_OBJECTS) $(INTERRUPT_OBJECTS) $(EXCEPTION_OBJECTS) $(SYSCALL_OBJECTS)
# 	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(STATICLIBS) $(OBJECTS_ASM) $(DRIVER_OBJECTS) $(SCHEDULER_OBJECTS) $(EVENT_HANDLING_OBJECTS) $(INTERRUPT_OBJECTS) $(EXCEPTION_OBJECTS) $(SYSCALL_OBJECTS)
$(KERNEL): $(LOADEROBJECT) $(OBJECTS) $(SUB_OBJECTS) $(OBJECTS_ASM)
	$(LD) $(LDFLAGS) -T kernel.ld -o $(KERNEL) $(LOADEROBJECT) $(OBJECTS) $(SUB_OBJECTS) $(OBJECTS_ASM)

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
