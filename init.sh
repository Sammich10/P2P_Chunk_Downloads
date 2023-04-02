#!/bin/bash

if [ $# -ne 2 ]; then
  echo "Usage: init.sh <type(a2a|tree)> <num super peers>"
  exit 1
fi
if [ $1 != "a2a" ] && [ $1 != "tree" ]; then
  echo "Usage: init.sh <type(a2a|tree)>"
  exit 1
fi

if [ $2 -lt 1 ] || [ $2 -gt 10 ]; then
  echo "Number of super peers must be between 1 and 10"
  exit 1
fi

p=$((${2}*3))

pkill -9 peernode.o
pkill -9 superpeer.o

make clean
rm logs/*
rm results/*
rm tests/*
rm peer_configs/super_peers/*
rm peer_configs/weak_peers/*
for i in $(seq 1 $p)
do
    rm peer_files/p${i}_files/*
done

make;

if [ ! -d "peer_files" ]; then
  mkdir "peer_files"
fi

if [ ! -d "peer_configs/weak_peers" ]; then
  mkdir "peer_configs/weak_peers"
fi

if [ ! -d "peer_configs/super_peers" ]; then
  mkdir "peer_configs/super_peers"
fi

echo Initializing peer configs...

for i in $(seq 1 $p)
do
  j=$((8082 + ${i}))
  echo "PEER_ID peer${i}" > "peer_configs/weak_peers/peer${i}.cfg"
  echo "IP 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
  echo "PORT ${j}" >> "peer_configs/weak_peers/peer${i}.cfg"
  echo "HOST_FOLDER peer_files/p${i}_files" >> "peer_configs/weak_peers/peer${i}.cfg"
  if (( i <= 3)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8060" >> "peer_configs/weak_peers/peer${i}.cfg"
  elif (( i <= 6)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8061" >> "peer_configs/weak_peers/peer${i}.cfg"
  elif (( i <= 9)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8062" >> "peer_configs/weak_peers/peer${i}.cfg"
  elif (( i <= 12)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8063" >> "peer_configs/weak_peers/peer${i}.cfg"
  elif (( i <= 15)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8064" >> "peer_configs/weak_peers/peer${i}.cfg"
  elif (( i <= 18)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8065" >> "peer_configs/weak_peers/peer${i}.cfg"
  elif (( i <= 21)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8066" >> "peer_configs/weak_peers/peer${i}.cfg"
  elif (( i <= 24)); then
    echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
    echo "SUPER_PEER_PORT 8067" >> "peer_configs/weak_peers/peer${i}.cfg"
  fi
done

echo "Initializing super peer configs..."

for i in $(seq 1 $2); do
  j=$((8059 + ${i}))
  echo "PEER_ID superpeer${i}" > "peer_configs/super_peers/superpeer${i}.cfg"
  echo "IP 127.0.0.1" >> "peer_configs/super_peers/superpeer${i}.cfg"
  echo "PORT ${j}" >> "peer_configs/super_peers/superpeer${i}.cfg"
done

sp=$((${2}-1))

if [ "$1" = "a2a" ]; then
  echo "Configuring super peers for all to all network..."
  for i in $(seq 0 $sp); do
  echo "NEIGHBOR 127.0.0.1:8059" >> "peer_configs/super_peers/superpeer$((i+1)).cfg"
    for j in $(seq 0 $sp); do
      if (( i != j )); then
      echo "NEIGHBOR 127.0.0.1:806$j" >> "peer_configs/super_peers/superpeer$((i+1)).cfg"
      fi
    done
  done
elif [ "$1" = "tree" ]; then
  echo "Configuring super peers for tree network..."

  echo "NEIGHBOR 127.0.0.1:8061" >> "peer_configs/super_peers/superpeer1.cfg"
  echo "NEIGHBOR 127.0.0.1:8062" >> "peer_configs/super_peers/superpeer1.cfg"

  echo "NEIGHBOR 127.0.0.1:8060" >> "peer_configs/super_peers/superpeer2.cfg"
  echo "NEIGHBOR 127.0.0.1:8063" >> "peer_configs/super_peers/superpeer2.cfg"
  echo "NEIGHBOR 127.0.0.1:8064" >> "peer_configs/super_peers/superpeer2.cfg"

  echo "NEIGHBOR 127.0.0.1:8065" >> "peer_configs/super_peers/superpeer3.cfg"
  echo "NEIGHBOR 127.0.0.1:8066" >> "peer_configs/super_peers/superpeer3.cfg"
  echo "NEIGHBOR 127.0.0.1:8060" >> "peer_configs/super_peers/superpeer3.cfg"

  echo "NEIGHBOR 127.0.0.1:8067" >> "peer_configs/super_peers/superpeer4.cfg"
  echo "NEIGHBOR 127.0.0.1:8068" >> "peer_configs/super_peers/superpeer4.cfg"
  echo "NEIGHBOR 127.0.0.1:8061" >> "peer_configs/super_peers/superpeer4.cfg"

  echo "NEIGHBOR 127.0.0.1:8069" >> "peer_configs/super_peers/superpeer5.cfg"
  echo "NEIGHBOR 127.0.0.1:8061" >> "peer_configs/super_peers/superpeer5.cfg"

  echo "NEIGHBOR 127.0.0.1:8062" >> "peer_configs/super_peers/superpeer6.cfg"

  echo "NEIGHBOR 127.0.0.1:8062" >> "peer_configs/super_peers/superpeer7.cfg"

  echo "NEIGHBOR 127.0.0.1:8063" >> "peer_configs/super_peers/superpeer8.cfg"

  echo "NEIGHBOR 127.0.0.1:8063" >> "peer_configs/super_peers/superpeer9.cfg"

  echo "NEIGHBOR 127.0.0.1:8064" >> "peer_configs/super_peers/superpeer10.cfg"
fi


echo Initializing peer files...

for i in $(seq 1 $p)
do
  if [ ! -d "peer_files/p${i}_files" ]
  then
    mkdir "peer_files/p${i}_files"
  fi

  cp "peer_files/peerfiles/t128b.txt" "peer_files/p${i}_files/t128b${i}.txt"
  cp "peer_files/peerfiles/t512b.txt" "peer_files/p${i}_files/t512b${i}.txt"
  cp "peer_files/peerfiles/t2kb.txt" "peer_files/p${i}_files/t2kb${i}.txt"
  cp "peer_files/peerfiles/t8kb.txt" "peer_files/p${i}_files/t8kb${i}.txt"
  cp "peer_files/peerfiles/t32kb.txt" "peer_files/p${i}_files/t32kb${i}.txt"

done

echo Initializing test files...

for i in {1..5}
do
  if [ ${i} -eq 1 ]
  then
    name="t128b"
    file="tests/test128b.txt"
  elif [ ${i} -eq 2 ]
  then
    name="t512b"
    file="tests/test512b.txt"
  elif [ ${i} -eq 3 ]
  then
    name="t2kb"
    file="tests/test2kb.txt"
  elif [ ${i} -eq 4 ]
  then
    name="t8kb"
    file="tests/test8kb.txt"
  elif [ ${i} -eq 5 ]
  then
    name="t32kb"
    file="tests/test32kb.txt"
  fi
  for j in {1..4}
  do
    echo "${name}${j}.txt" >> "${file}"
  done
done

if [ ! -d "tests" ]; then
  mkdir "tests"
fi

for i in $(seq 1 $p)
do
  echo "t128b${i}.txt">> "tests/test${i}.txt"
  echo "t512b${i}.txt">> "tests/test${i}.txt"
  echo "t2kb${i}.txt">> "tests/test${i}.txt"
  echo "t8kb${i}.txt">> "tests/test${i}.txt"
  echo "t32kb${i}.txt">> "tests/test${i}.txt"
done

if [ ! -d "logs" ]; then
  mkdir "logs"
fi
