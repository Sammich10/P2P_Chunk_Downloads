./init.sh

make

./server.o peer_configs/server.cfg > logs/server.log &

sleep 2

echo "starting 8 peer nodes, they will be alive for 100 seconds then shut themselves down. Be sure to kill the server after you're finished..."

cat tests/wait100.txt tests/exit.txt | ./peernode.o peer_configs/peer1.cfg > logs/peer1.log &

cat tests/wait100.txt tests/exit.txt | ./peernode.o peer_configs/peer2.cfg > logs/peer2.log &

cat tests/wait100.txt tests/exit.txt | ./peernode.o peer_configs/peer3.cfg > logs/peer3.log &

cat tests/wait100.txt tests/exit.txt | ./peernode.o peer_configs/peer4.cfg > logs/peer4.log &

cat tests/wait100.txt tests/exit.txt | ./peernode.o peer_configs/peer5.cfg > logs/peer5.log &

cat tests/wait100.txt tests/exit.txt | ./peernode.o peer_configs/peer6.cfg > logs/peer6.log &

cat tests/wait100.txt tests/exit.txt | ./peernode.o peer_configs/peer7.cfg > logs/peer7.log &

./peernode.o peer_configs/peer8.cfg 


