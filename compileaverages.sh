RESULTS_DIR="results/"
RESULTS_FOLDER="results"
col=1

for csv_file in $(find $RESULTS_DIR -name "*.csv"); do
    echo "Processing $csv_file"
    #python3 plot.py $csvf_ile
    avg=$(awk -F ',' '{ sum += $'$col'; n++ } END { if (n > 0) print sum / n }' $csv_file)
    echo "$csv_file,Average response time: $avg" >> $RESULTS_FOLDER/averages.txt
done
