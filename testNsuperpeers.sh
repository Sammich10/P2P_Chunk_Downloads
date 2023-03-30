#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Usage: $0 <num super peers>"
  exit 1
fi

if [ $1 -lt 1 ] || [ $1 -gt 10 ]; then
  echo "Number of super peers must be between 1 and 10"
  exit 1
fi

if [ ! -d "logs" ]; then
  mkdir "logs"
fi
if [ ! -d "tests" ]; then
  echo "test files not found, try running init.sh"
  exit 1
fi
if [ ! -d "results" ]; then
  mkdir "results"
fi


# Create an array of numbers 1 to # of super peers
numbers=($(seq 1 $((3 * $1))))

# Shuffle the array randomly
shuf_numbers=($(shuf -e "${numbers[@]}"))

# Create a new array to store the result
result=()

# Iterate through the shuffled array and add each number to the result
# array, skipping the index that matches the number
for i in "${!shuf_numbers[@]}"; do
  if [[ "${shuf_numbers[i]}" -ne "$i"+1 ]]; then
    result+=("${shuf_numbers[i]}")
  else
    # Find the first number in the shuffled array that is not at its correct
    # index, and swap it with the current number
    for j in "${!shuf_numbers[@]}"; do
      if [[ "${shuf_numbers[j]}" -ne "$j"+1 ]] && [[ "${shuf_numbers[j]}" -ne "${shuf_numbers[i]}" ]]; then
        temp="${shuf_numbers[j]}"
        shuf_numbers[j]="${shuf_numbers[i]}"
        shuf_numbers[i]="$temp"
        result+=("${shuf_numbers[i]}")
        break
      fi
    done
  fi
done


echo "Starting ${1} super peers"

for i in $(seq 1 $1); do
  ./superpeer.o peer_configs/super_peers/superpeer${i}.cfg > logs/superpeer${i}.log 2>&1 &
done

sleep $1

j=$((3 * ${1}))

echo "Starting ${j} peers"

for x in $(seq 1 $j); do
  ./peernode.o peer_configs/weak_peers/peer${x}.cfg tests/test${result[$x-1]}.txt > logs/peer${x}.log 2>&1 &
done

