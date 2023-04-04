#!/bin/bash

if [ $# -ne 1 ]; then
  echo "Usage: $0 <result file name>"
  exit 1
fi

LOG_DIR="logs/"
LOG_FILES=$(find $LOG_DIR -name "*.log")
RESULTS_FOLDER="results"

if [ ! -d $RESULTS_FOLDER ]; then
  mkdir $RESULTS_FOLDER
fi

for LOG_FILE in $LOG_FILES; do
  while read LINE; do
    if [[ $LINE =~ response\ time:\ ([0-9]+\.[0-9]+) ]]; then
      RESPONSE_TIME=${BASH_REMATCH[1]}
      echo "$RESPONSE_TIME," >> $RESULTS_FOLDER/$1.csv
    fi
  done < $LOG_FILE
done
