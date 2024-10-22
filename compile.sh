docker start TPE-Arqui
docker exec -u $USER -it TPE-Arqui make clean -C /root/Toolchain
docker exec -u $USER -it TPE-Arqui make clean -C /root/
docker exec -u $USER -it TPE-Arqui make -C /root/Toolchain
docker exec -u $USER -it TPE-Arqui make -C /root/
docker stop TPE-Arqui
