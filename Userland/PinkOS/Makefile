# include ../Makefile.inc

# MODULE=0000-PinkOS.bin
# SOURCES=$(wildcard [^_]*.c)

# all: $(MODULE)

# $(MODULE): $(SOURCES)
# 	$(GCC) $(GCCFLAGS) -T PinkOS.ld _loader.c $(SOURCES) -o ../$(MODULE)

# clean:
# 	rm -rf *.o

# .PHONY: all clean print

include ../Makefile.inc

MODULE=0000-PinkOS.bin
SOURCES=$(wildcard [^_]*.c)
SOURCES_ASM=$(wildcard asm/*.asm)
# PROGRAM_SOURCES=$(wildcard programs/*.c)
SUB_SOURCES=$(wildcard */*.c)

OBJECTS=$(SOURCES:.C=.o)
SUB_OBJECTS=$(SUB_SOURCES:.c=.o)
OBJECTS_ASM=$(SOURCES_ASM:.asm=.o)
STATICCLIBS=

# INCLUDES = -Iinclude -Iinclude/libs -Iinclude/syscalls
INCLUDES = $(foreach dir,$(shell find . -type d),-I$(dir))
all: $(MODULE)

$(MODULE): _loader.c $(OBJECTS) $(SUB_OBJECTS) $(OBJECTS_ASM)
	$(GCC) $(GCCFLAGS) -T PinkOS.ld -o ../$(MODULE) _loader.c $(OBJECTS) $(SUB_OBJECTS) $(OBJECTS_ASM) $(STATICLIBS) $(INCLUDES)

%.o: %.c
	$(GCC) $(GCCFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.asm
	$(ASM) $(ASMFLAGS) $< -o $@

clean:
	find . -type f -name '*.o' -delete