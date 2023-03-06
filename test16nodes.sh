#!/bin/bash

./init.sh

if [ ! -d "logs" ]; then
  mkdir "logs"
fi
if [ ! -d "tests" ]; then
  echo "test files not found"
  exit 1
fi

./server.o peer_configs/server.cfg > logs/server.log &

sleep 5

cat tests/wait20.txt tests/test5.txt tests/test2.txt tests/exit.txt | ./peernode.o peer_configs/peer1.cfg > logs/peer1.log &

cat tests/wait20.txt tests/test3.txt tests/test8.txt tests/exit.txt | ./peernode.o peer_configs/peer2.cfg > logs/peer2.log &

cat tests/wait20.txt tests/test16.txt tests/test1.txt tests/exit.txt | ./peernode.o peer_configs/peer3.cfg > logs/peer3.log &

cat tests/wait20.txt tests/test7.txt tests/test12.txt tests/exit.txt | ./peernode.o peer_configs/peer4.cfg > logs/peer4.log &

cat tests/wait20.txt tests/test1.txt tests/test13.txt tests/exit.txt | ./peernode.o peer_configs/peer5.cfg > logs/peer5.log &

cat tests/wait20.txt tests/test12.txt tests/test5.txt tests/exit.txt | ./peernode.o peer_configs/peer6.cfg > logs/peer6.log &

cat tests/wait20.txt tests/test13.txt tests/test8.txt tests/exit.txt | ./peernode.o peer_configs/peer7.cfg > logs/peer7.log &

cat tests/wait20.txt tests/test1.txt tests/test3.txt tests/exit.txt | ./peernode.o peer_configs/peer8.cfg > logs/peer8.log &

cat tests/wait20.txt tests/test2.txt tests/test5.txt tests/exit.txt | ./peernode.o peer_configs/peer9.cfg > logs/peer9.log &

cat tests/wait20.txt tests/test3.txt tests/test8.txt tests/exit.txt | ./peernode.o peer_configs/peer10.cfg > logs/peer10.log &

cat tests/wait20.txt tests/test6.txt tests/test1.txt tests/exit.txt | ./peernode.o peer_configs/peer11.cfg > logs/peer11.log &

cat tests/wait20.txt tests/test7.txt tests/test11.txt tests/exit.txt | ./peernode.o peer_configs/peer12.cfg > logs/peer12.log &

cat tests/wait20.txt tests/test9.txt tests/test3.txt tests/exit.txt | ./peernode.o peer_configs/peer13.cfg > logs/peer13.log &

cat tests/wait20.txt tests/test12.txt tests/test5.txt tests/exit.txt | ./peernode.o peer_configs/peer14.cfg > logs/peer14.log &

cat tests/wait20.txt tests/test14.txt tests/test8.txt tests/exit.txt | ./peernode.o peer_configs/peer15.cfg > logs/peer15.log &

cat tests/wait20.txt tests/test11.txt tests/test3.txt tests/exit.txt | ./peernode.o peer_configs/peer16.cfg > logs/peer16.log &


sleep  1
echo "testing, please wait... This will take about 60 seconds"
sleep 50
echo "5"
sleep 1
echo "4"
sleep 1
echo "3"
sleep 1
echo "2"
sleep 1
echo "1"
sleep 1

echo "killing all processes..."

pkill -f ./peernode.o
sleep 5
pkill -f ./server.o


echo "test finished, check logs and results folder"

if [ -e "results/16_node_response_times.csv" ]; then
  rm results/16_node_response_times.csv
fi

for i in {1..16}
do
  awk '/Time taken to find peers:/ {gsub(/seconds/, ""); print $NF}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/16_node_response_times.csv
done
: << COMMENT
if [ -e "results/16_node_download_times.csv" ]; then
  rm results/16_node_download_times.csv
fi

for i in {1..16}
do
  awk '/Received/ {bytes=$2; time=$5; gsub(/seconds/, "", time); print bytes","time}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/16_node_download_times.csv
done

if [ -e "results/16_node_upload_times.csv" ]; then
  rm results/16_node_upload_times.csv
fi

for i in {1..16}
do
  awk '/Sent/ {bytes=$2; time=$5; gsub(/seconds/, "", time); print bytes","time}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/16_node_upload_times.csv
done
COMMENT
