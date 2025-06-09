docker start TP2-SO
docker exec -u $USER -it TP2-SO make clean -C /root/Toolchain
docker exec -u $USER -it TP2-SO make clean -C /root/
docker exec -u $USER -it TP2-SO make -C /root/Toolchain
docker exec -u $USER -it TP2-SO make -C /root/
# docker stop TPE-Arqui
