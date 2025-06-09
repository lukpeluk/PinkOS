docker pull agodio/itba-so-multi-platform:3.0
docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name TP2-SO agodio/itba-so-multi-platform:3.0

docker start TP2-SO

# Crear usuario con mismo UID/GID que el host
docker exec TP2-SO groupadd -g $(id -g) -o $USER 2>/dev/null || true
docker exec TP2-SO useradd -m -u $(id -u) -g $(id -g) -o -s /bin/bash $USER 2>/dev/null || true
docker exec TP2-SO chown -R $USER:$USER /root

# Configurar para que los comandos se ejecuten como el usuario correcto
echo "Para ejecutar comandos, usa: docker exec -u $USER TP2-SO <comando>"
echo "O para una sesión interactiva: docker exec -u $USER -it TP2-SO bash"
