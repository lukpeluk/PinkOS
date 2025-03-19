#!/bin/bash

# Directorio de los archivos fuente
SRC_DIR="./programs"

# Archivos fuente
SOURCES="$SRC_DIR/audioLib.c $SRC_DIR/graphicsLib.c $SRC_DIR/keyboardLib.c $SRC_DIR/stdpink.c $SRC_DIR/test.c"

# Archivos de cabecera
HEADERS="$SRC_DIR/audioLib.h $SRC_DIR/graphicsLib.h $SRC_DIR/keyboard.h $SRC_DIR/stdpink.h $SRC_DIR/syscallCodes.h"

# Archivo de salida
OUTPUT="program"

# Compilar y enlazar
gcc -o $OUTPUT $SOURCES

# Verificar si la compilación fue exitosa
if [ $? -eq 0 ]; then
    echo "Compilación y enlace exitosos. Ejecutable generado: $OUTPUT"
else
    echo "Error en la compilación o el enlace."
fi