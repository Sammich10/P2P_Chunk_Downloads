#!/bin/bash

for N in $(seq 1 $1)
do
    ruby clienttest.rb &
done

wait
