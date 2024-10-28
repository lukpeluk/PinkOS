docker pull agodio/itba-so:2.0
docker run -d -v ${PWD}:/root --security-opt seccomp:unconfined -it --name TPE-Arqui agodio/itba-so:2.0

docker start TPE-Arqui

docker exec TPE-Arqui groupadd -g 1000 -o $USER
docker exec TPE-Arqui useradd -m -u 1000 -g 1000 -o -s /bin/bash $USER
