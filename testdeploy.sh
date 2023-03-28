#!/bin/bash

make clean; make

if [ ! -d "logs" ]; then
  mkdir "logs"
fi
if [ ! -d "tests" ]; then
  echo "test files not found"
  exit 1
fi
if [ ! -d "results" ]; then
  mkdir "results"
fi

./server.o peer_configs/server.cfg > logs/server.log &

sleep 5
: '
./peernode.o peer_configs/peer1.cfg < tests/test3.txt < tests/test2.txt < tests/exit.txt > logs/peer1.log &

./peernode.o peer_configs/peer2.cfg < tests/test1.txt < tests/test3.txt < tests/exit.txt > logs/peer2.log &

./peernode.o peer_configs/peer3.cfg < tests/test2.txt <tests/test1.txt < tests/exit.txt > logs/peer3.log &
'

cat tests/test3.txt tests/test2.txt tests/exit.txt | ./peernode.o peer_configs/peer1.cfg > logs/peer1.log &

cat tests/test1.txt tests/test3.txt tests/exit.txt | ./peernode.o peer_configs/peer2.cfg > logs/peer2.log &

cat tests/test2.txt tests/test1.txt tests/exit.txt | ./peernode.o peer_configs/peer3.cfg > logs/peer3.log &


sleep  1
echo "testing, please wait..."
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

pkill -f ./peernode.o
sleep 2
pkill -f ./server.o
