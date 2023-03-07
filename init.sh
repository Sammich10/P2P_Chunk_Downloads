#!/bin/bash

if [ ! -d "peer_files" ]; then
  exit
fi

if [ ! -d "peer_configs" ]; then
  mkdir "peer_configs"
  for {i..16}
  do
    j = ( 8082 + i )
    echo "peer${i}" > "peer_configs/peer${i}.cfg"
    echo

  done
fi

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

for i in {1..16}
do
  echo "wait 3" > "tests/test${i}.txt"
  echo "t128b${i}.txt">> "tests/test${i}.txt"
  echo "t512b${i}.txt">> "tests/test${i}.txt"
  echo "t2kb${i}.txt">> "tests/test${i}.txt"
  echo "t8kb${i}.txt">> "tests/test${i}.txt"
  echo "t32kb${i}.txt">> "tests/test${i}.txt"
  echo "update" >> "tests/test${i}.txt"
  echo "wait 4" >> "tests/test${i}.txt"
done

echo "exit" >> "tests/exit.txt"

if [ ! -d "logs" ]; then
  mkdir "logs"
fi
rm logs/*

