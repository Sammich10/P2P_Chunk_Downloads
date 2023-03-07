#!/bin/bash

./init.sh

if [ ! -d "logs" ]; then
  mkdir "logs"
fi
if [ ! -d "tests" ]; then
  echo "test files not found"
  exit 1
fi

echo "starting server"
./server.o peer_configs/server.cfg > logs/server.log &

sleep 5

echo "starting 3 peer nodes"

cat tests/wait10.txt tests/test2.txt tests/test3.txt tests/exit.txt | ./peernode.o peer_configs/peer1.cfg > logs/peer1.log &

cat tests/wait10.txt tests/test1.txt tests/test3.txt tests/exit.txt | ./peernode.o peer_configs/peer2.cfg > logs/peer2.log &

cat tests/wait10.txt tests/test1.txt tests/test2.txt tests/exit.txt | ./peernode.o peer_configs/peer3.cfg > logs/peer3.log &


sleep  1
echo "testing, please wait... This will take about 20 seconds"
sleep 15
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

if [ -e "results/  3_node_response_times.csv" ]; then
  rm results/  3_node_response_times.csv
fi

for i in {1..3}
do
  awk '/Time taken to find peers:/ {gsub(/seconds/, ""); print $NF}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/3_node_response_times.csv
done
: << COMMENT
if [ -e "results/  3_node_download_times.csv" ]; then
  rm results/  3_node_download_times.csv
fi

for i in {1..  3}
do
  awk '/Received/ {bytes=$2; time=$5; gsub(/seconds/, "", time); print bytes","time}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/  3_node_download_times.csv
done

if [ -e "results/  3_node_upload_times.csv" ]; then
  rm results/  3_node_upload_times.csv
fi

for i in {1..  3}
do
  awk '/Sent/ {bytes=$2; time=$5; gsub(/seconds/, "", time); print bytes","time}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/  3_node_upload_times.csv
done
COMMENT
