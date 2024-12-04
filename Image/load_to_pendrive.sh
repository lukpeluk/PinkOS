#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Path to the image file, relative to the script location
IMAGE_PATH="$SCRIPT_DIR/x64BareBonesImage.img"


if [ "$1" == "-h" ]; then
  echo "Permite generar un pendrive booteable con PinkOS, OJO que te formatea el disco/pendrive que uses."
  echo "Si no sabés el nombre del pendrive correlo con la opción -l y te los lista."
  echo
  echo "Usage: $0 <disk> [option]"
  echo "Options:"
  echo "  -l       Lista los discos usando lsblk (para ver el nombre del pendrive donde cargar la imagen)."
  echo "  -e	   Permite extraer el disco de forma segura"
  echo "  -t       Testea el pendrive con QEMU, para no tener que reiniciar la compu. (no tiene todas las cosas fancys como audio y eso, es quemu pelado sin flags)."
  echo "  -d       Usa el nombre de disco por defecto (sda), en el caso de -t puede omitirse y es lo mismo"
  echo
  echo
  echo "ejemplos de casos de uso:"
  echo "    ... sdx -t 	testea el disco sdx," 
  echo "    ... -t 	testea el disco por defecto," 
  echo "    ... -d -t 	también testea el disco por defecto," 
  echo "    ... -e 	permite extraer el disco por defecto," 
  echo "    ... sdx  	carga la imágen en el disco sdx," 
  echo "    ... -d 	carga la imágen en el disco por defecto," 
  echo "    ... 		tira error (por seguridad no usa el por defecto en este caso, si no capaz te formatea un disco sin querer)." 
  exit 0
fi

# no args
if [ $# -eq 0 ]; then
  echo "Usage: $0 <disk> [option]"
  echo "Usa '-h' para ver las opciones disponibles."
  exit 1
fi


if [ "$1" == "-l" ]; then
  # Run lsblk to list block devices
  lsblk
  exit 0
fi

# Check if an argument is provided or if it's '-d'
if [ "$1" == "-d" ] || [ "$1" == "-e" ] || [ "$1" == "-t" ] || [ -z "$1" ]; then
  DISK="sda"  # Default to sda
else
  DISK="$1"    # Use the provided argument
fi

if [ "$1" == "-t" ] || [ "$2" == "-t" ]; then
  # Uses quemu to test the image (it doesn't support audio or anything fancy yet, maybe it could be integrated with ./run)
  sudo qemu-system-x86_64 -drive file=/dev/$DISK,format=raw 
  exit 0
fi

if [ "$1" == "-e" ] || [ "$2" == "-e" ]; then
  lsblk -rno MOUNTPOINT /dev/$DISK | xargs -r sudo umount
  udisksctl power-off -b /dev/$DISK
  exit 0
fi

# Run dd to copy the image to the specified disk
sudo dd if="$IMAGE_PATH" of=/dev/$DISK bs=4M status=progress

# Sync to ensure data is written
sudo sync

