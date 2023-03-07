#!/bin/bash

if [ ! -d "logs" ]; then
  mkdir "logs"
fi
if [ ! -d "tests" ]; then
  echo "test files not found"
  exit 1
fi

if [ -e "results/varyingsize_upload_times.csv" ]; then
  rm results/varyingsize_upload_times.csv
fi

if [ -e "results/varyingsize_download_times.csv" ]; then
  rm results/varyingsize_download_times.csv
fi

file="tests/test128b.txt"

rm logs/*

for i in {1..5}

do
    ./init.sh
    if [ ${i} -eq 1 ]
    then
        echo "testing 128 bytes file"
        file="tests/test128b.txt"
    elif [ ${i} -eq 2 ]
    then
        echo "testing 512 bytes file"
        file="tests/test512b.txt"
    elif [ ${i} -eq 3 ]
    then
        echo "testing 2 kilobytes file"
        file="tests/test2kb.txt"
    elif [ ${i} -eq 4 ]
    then
        echo "testing 8 kilobytes file"
        file="tests/test8kb.txt"
    elif [ ${i} -eq 5 ]
    then
        echo "testing 32 kilobytes file"
        file="tests/test32kb.txt"
    fi

    echo "creating server"
    
    ./server.o peer_configs/server.cfg > logs/server.log &

    sleep 5

    cat tests/wait10.txt ${file} ${file} ${file} tests/exit.txt | ./peernode.o peer_configs/peer1.cfg >> logs/peer1.log &

    cat tests/wait10.txt ${file} ${file} ${file} tests/exit.txt | ./peernode.o peer_configs/peer2.cfg >> logs/peer2.log &

    cat tests/wait10.txt ${file} ${file} ${file} tests/exit.txt | ./peernode.o peer_configs/peer3.cfg >> logs/peer3.log &

    cat tests/wait10.txt ${file} ${file} ${file} tests/exit.txt | ./peernode.o peer_configs/peer4.cfg >> logs/peer4.log &

    sleep  1
    echo "testing, please wait... This will take about 30 seconds"
    sleep 30

    pkill -f ./peernode.o
    sleep 2
    pkill -f ./server.o
    sleep 2


    for i in {1..4}
    do
        awk '/Received/ {bytes=$2; time=$5; gsub(/seconds/, "", time); print bytes","time}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/varyingsize_download_times.csv
    done

    for i in {1..4}
    do
        awk '/Sent/ {bytes=$2; time=$5; gsub(/seconds/, "", time); print bytes","time}' logs/peer${i}.log | awk '{printf("%s,\n",$0)}' >> results/varyingsize_upload_times.csv
    done

done

echo "test finished, check logs and results folder"