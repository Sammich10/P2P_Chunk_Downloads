#!/bin/bash

if [ $# -ne 3 ]; then
  echo "Usage: init.sh <type(a2a|tree)> <num super peers> <num peers per super peer>"
  exit 1
fi
if [ $1 != "a2a" ] && [ $1 != "tree" ]; then
  echo "Usage: init.sh <type(a2a|tree)> <num super peers> <num peers per super peer>"
  exit 1
fi

if [ $2 -lt 1 ] || [ $2 -gt 10 ]; then
  echo "Number of super peers must be between 1 and 10"
  exit 1
fi

if [ $3 -lt 1 ] || [ $3 -gt 5 ]; then
  echo "Number of peers per super peer must be between 1 and 5"
  exit 1
fi

p=$((${2}*${3}))

pkill -9 peernode.o
pkill -9 superpeer.o

rm logs/*
rm tests/*
rm peer_configs/super_peers/*
rm peer_configs/weak_peers/*

if [ ! -d "peer_files" ]; then
  mkdir "peer_files"
fi

if [ ! -d "peer_configs/weak_peers" ]; then
  mkdir "peer_configs/weak_peers"
fi

if [ ! -d "peer_configs/super_peers" ]; then
  mkdir "peer_configs/super_peers"
fi

for i in {1..50}; do
  if [ -d "peer_files/p${i}_files" ]; then
    rm -r "peer_files/p${i}_files"
  fi
done

echo Initializing peer configs...

k=8059
for i in $(seq 1 $p); do
  j=$((8082 + ${i}))
  echo "PEER_ID peer${i}" > "peer_configs/weak_peers/peer${i}.cfg"
  echo "IP 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
  echo "PORT ${j}" >> "peer_configs/weak_peers/peer${i}.cfg"
  echo "HOST_FOLDER peer_files/p${i}_files" >> "peer_configs/weak_peers/peer${i}.cfg"
  if (( (i-1) % $3 == 0)); then
    k=$((k + 1))
  fi
  echo "SUPER_PEER_ADDR 127.0.0.1" >> "peer_configs/weak_peers/peer${i}.cfg"
  echo "SUPER_PEER_PORT ${k}" >> "peer_configs/weak_peers/peer${i}.cfg"
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
  #echo "NEIGHBOR 127.0.0.1:8059" >> "peer_configs/super_peers/superpeer$((i+1)).cfg"
    for j in $(seq 0 $sp); do
      if (( i != j )); then
      echo "NEIGHBOR 127.0.0.1:806$j" >> "peer_configs/super_peers/superpeer$((i+1)).cfg"
      fi
    done
  done
elif [ "$1" = "tree" ]; then
  echo "Configuring super peers for tree network..."
  total_nodes=$2
  port_base=8060
  #echo "NEIGHBOR 127.0.0.1:8059" >> "peer_configs/super_peers/superpeer1.cfg"
  for ((i=0; i<$total_nodes; i++)); do
    node_num=$((i+1))
    cfg_file="peer_configs/super_peers/superpeer$node_num.cfg"

    # Write NEIGHBOR entries for left and right children
    left_child=$((2*i+1))
    if ((left_child < total_nodes)); then
      left_port=$((port_base+left_child))
      echo "NEIGHBOR 127.0.0.1:$left_port" >> "$cfg_file"
    fi

    right_child=$((2*i+2))
    if ((right_child < total_nodes)); then
      right_port=$((port_base+right_child))
      echo "NEIGHBOR 127.0.0.1:$right_port" >> "$cfg_file"
    fi

    # Write NEIGHBOR entry for parent, if not root
    if ((i > 0)); then
      parent=$(( (i-1)/2 ))
      parent_port=$((port_base+parent))
      echo "NEIGHBOR 127.0.0.1:$parent_port" >> "$cfg_file"
    fi
  done
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
