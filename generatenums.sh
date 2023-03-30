#!/bin/bash

# Create an array of numbers 1 to 10
numbers=($(seq 1 $1))

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

# Print the result array
echo "${result[@]}"
