# Estalber preferencias de visualización
set disassembly-flavor intel
set print pretty on
set print address on
set pagination off
layout src
layout regs

# Conectar al servidor GDB
target remote localhost:1234

# Cargar el kernel
add-symbol-file Kernel/kernel.elf 0x100000


# Cargar el binario de userland
add-symbol-file Userland/0000-PinkOS.elf 0x400000

directory Kernel/src \
          Kernel/src/interrupts \
          Kernel/src/processManager \
          Kernel/src/eventManager \
          Kernel/src/syscalls \
          Kernel/src/drivers \
          Kernel/src/exceptions \
          Kernel/src/fileSystem \
          Kernel/src/memoryManager \
          Kernel/src/processManager \
          Kernel/src/programManager \
          Kernel/src/windowManager \
          Kernel/asm \
          .
directory Userland/src \
            Userland/PinkOS \
            Userland/PinkOS/libs \
            Userland/PinkOS/programs \
            Userland/PinkOS/asm

# Prints
echo "Cargando kernel y userland...\n"

# Configurar puntos de interrupción iniciales
# break kernel.c:316
break shell.c:503
# break interrupts.asm:137
