

#!/bin/bash

# Verificar si el contenedor TP2-SO existe
if ! docker ps -a --format "table {{.Names}}" | grep -q "^TP2-SO$"; then
    echo "Contenedor TP2-SO no encontrado. Ejecutando configuración inicial..."
    if [ -f ".first_time.sh" ]; then
        bash .first_time.sh
    else
        echo "Error: .first_time.sh no encontrado"
        exit 1
    fi
else
    echo "Contenedor TP2-SO encontrado. Iniciando..."
    docker start TP2-SO
fi

# Compilar el proyecto
docker exec -u $USER -it TP2-SO make clean -C /root/Toolchain
docker exec -u $USER -it TP2-SO make clean -C /root/
docker exec -u $USER -it TP2-SO make -C /root/Toolchain
docker exec -u $USER -it TP2-SO make -C /root/
# docker stop TPE-Arqui
