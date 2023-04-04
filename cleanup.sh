#!/bin/bash

pkill -9 peernode.o
pkill -9 superpeer.o

make clean
rm logs/*
rm results/*
rm tests/*
rm peer_configs/super_peers/*
rm peer_configs/weak_peers/*
for i in {1..50}; do
  if [ -d "peer_files/p${i}_files" ]; then
    rm -r "peer_files/p${i}_files"
  fi
done