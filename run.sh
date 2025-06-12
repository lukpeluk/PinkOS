#!/bin/bash

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  # Para Linux
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 2G \
    -audiodev pa,id=speaker \
    -machine pcspk-audiodev=speaker \
    -serial tcp::4444,server,nowait
    # -rtc base=localtime \

    # -serial tcp::4444,server,nowait
elif [[ "$OSTYPE" == "darwin"* ]]; then
  # Para Mac
  qemu-system-x86_64 \
    -hda Image/x64BareBonesImage.qcow2 \
    -m 512 \
    -audiodev coreaudio,id=speaker \
    -machine pcspk-audiodev=speaker \
    -serial tcp::4444,server,nowait
else
  echo "Sistema operativo no soportado"
  exit 1
fi
