#!/bin/bash

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

echo "Starting 3 super peers"

./superpeer.o peer_configs/super_peers/superpeer1.cfg > logs/superpeer1.log &
./superpeer.o peer_configs/super_peers/superpeer2.cfg > logs/superpeer2.log &
./superpeer.o peer_configs/super_peers/superpeer3.cfg > logs/superpeer3.log &



sleep 5

echo "Starting 9 peers"

./peernode.o peer_configs/weak_peers/peer1.cfg > logs/peer1.log &
./peernode.o peer_configs/weak_peers/peer2.cfg > logs/peer2.log &
./peernode.o peer_configs/weak_peers/peer3.cfg > logs/peer3.log &
./peernode.o peer_configs/weak_peers/peer4.cfg > logs/peer4.log &
./peernode.o peer_configs/weak_peers/peer5.cfg > logs/peer5.log &
./peernode.o peer_configs/weak_peers/peer6.cfg > logs/peer6.log &
./peernode.o peer_configs/weak_peers/peer7.cfg > logs/peer7.log &
./peernode.o peer_configs/weak_peers/peer8.cfg > logs/peer8.log &
./peernode.o peer_configs/weak_peers/peer9.cfg > logs/peer9.log &
