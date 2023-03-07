#!/bin/bash

if [ ! -d "peer_files" ]; then
  exit
fi

if [ ! -d "peer_configs" ]; then
  mkdir "peer_configs"
  for i in {1..16}
  do
    j=$((8082 + ${i}))
    echo "peer${i}" > "peer_configs/peer${i}.cfg"
    echo "PORT ${j}" >> "peer_configs/peer${i}.cfg"

  done
  echo "PORT 8080" > "peer_configs/server.cfg"
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

echo "wait 10" > "tests/wait10.txt"
echo "wait 20" > "tests/wait20.txt"
echo "wait 100" > "tests/wait100.txt"
echo "wait 10" > "tests/exit.txt"
echo "exit" >> "tests/exit.txt"

if [ ! -d "logs" ]; then
  mkdir "logs"
fi

rm logs/*