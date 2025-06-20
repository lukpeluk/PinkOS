#!/bin/bash

# Verificar si se pasó el argumento -d para debug
DEBUG_MODE=false
if [[ "$1" == "-d" ]]; then
  DEBUG_MODE=true
  echo "Modo debug activado - GDB habilitado"
else
  echo "Modo normal - GDB deshabilitado"
fi

# Configurar flags de debug para GDB
if [[ "$DEBUG_MODE" == true ]]; then
  GDB_FLAGS="-s -S"
else
  GDB_FLAGS=""
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  # Para Linux
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 2G \
    -audiodev pa,id=speaker \
    -machine pcspk-audiodev=speaker \
    -serial tcp::4444,server \
    -no-hpet \
    -smp cores=4 \
    -enable-kvm \
    -cpu host \
    $GDB_FLAGS
    # -rtc base=localtime \
    # -serial tcp::4444,server,nowait
elif [[ "$OSTYPE" == "darwin"* ]]; then
  # Para Mac
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 2G \
    -audiodev coreaudio,id=speaker \
    -machine pcspk-audiodev=speaker \
    -serial tcp::4444,server \
    -smp cores=4 \
    $GDB_FLAGS
else
  echo "Sistema operativo no soportado"
  exit 1
fi
