docker pull agodio/itba-so:2.0
docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name TPE-Arqui agodio/itba-so:2.0

docker start TPE-Arqui
docker exec -it TPE-Arqui bash

groupadd -g 1000 -o $USER
useradd -m -u 1000 -g 1000 -o -s /bin/bash $USER
