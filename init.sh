#!/bin/bash

if [ ! -d "peer_configs" ]; then
  mkdir "peer_configs"
  for i in {1..16}
  do
    ip="127.0.0.1"
    j=$((8081 + i))
    echo "PORT ${j}" >> "peer_configs/peer${i}.cfg"
    echo "SERVER_IP ${ip}" >> "peer_configs/peer${i}.cfg"
    echo "HOST_FOLDER peer_files/p${i}_files" >> "peer_configs/peer${i}.cfg"

  done
fi

echo "PORT 8080" > "peer_configs/server.cfg"

for i in {1..16}
do
  if [ ! -d "peer_files/p${i}_files" ]
  then
    mkdir "peer_files/p${i}_files"
  fi
  
  rm peer_files/p${i}_files/*

  cp "peer_files/peerfiles/t128b.txt" "peer_files/p${i}_files/t128b${i}.txt"
  cp "peer_files/peerfiles/t512b.txt" "peer_files/p${i}_files/t512b${i}.txt"
  cp "peer_files/peerfiles/t2kb.txt" "peer_files/p${i}_files/t2kb${i}.txt"
  cp "peer_files/peerfiles/t8kb.txt" "peer_files/p${i}_files/t8kb${i}.txt"
  cp "peer_files/peerfiles/t32kb.txt" "peer_files/p${i}_files/t32kb${i}.txt"

done

if [ ! -d "tests" ]; then
  mkdir "tests"
fi

rm tests/*

echo "wait 10" > tests/wait10.txt

echo "wait 20" > tests/wait20.txt

echo "wait 100" > tests/wait100.txt

for i in {1..16}
do
  echo "wait 3" > "tests/test${i}.txt"
  echo "t128b${i}.txt">> "tests/test${i}.txt"
  echo "t512b${i}.txt">> "tests/test${i}.txt"
  echo "t2kb${i}.txt">> "tests/test${i}.txt"
  echo "t8kb${i}.txt">> "tests/test${i}.txt"
  echo "t32kb${i}.txt">> "tests/test${i}.txt"
  echo "update" >> "tests/test${i}.txt"
  echo "wait 3" >> "tests/test${i}.txt"
done

for i in {1..4}
do
  echo "t128b${i}.txt">> "tests/test128b.txt"
  echo "t512b${i}.txt">> "tests/test512b.txt"
  echo "t2kb${i}.txt">> "tests/test2kb.txt"
  echo "t8kb${i}.txt">> "tests/test8kb.txt"
  echo "t32kb${i}.txt">> "tests/test32kb.txt"
done

echo "wait 10" > "tests/exit.txt"
echo "exit" >> "tests/exit.txt"

if [ ! -d "logs" ]; then
  mkdir "logs"
fi

if [ ! -d "results" ]; then
  mkdir "results"
fi

rm logs/*
