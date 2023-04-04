j=1
t="a2a"

make clean; make;

for x in {1..2}; do
    if [ $x -eq 1 ]; then
        t="a2a"
    else
        t="tree"
    fi
    for i in {1..3}; do
        if [ $i -eq 1 ]; then
            j=1
        elif [ $i -eq 2 ]; then
            j=3
        else
            j=5
        fi
        ./test.sh $t 3 $j
        sleep 45
        echo "getting results for 3 super peers and $j peers per super peer"
        ./getresults.sh ${t}3superpeer${j}peer_resptime
    done 

    for i in {1..3}; do
        if [ $i -eq 1 ]; then
            j=1
        elif [ $i -eq 2 ]; then
            j=3
        else
            j=5
        fi
        ./test.sh $t 6 $j
        sleep 90
        echo "getting results for 6 super peers and $j peers per super peer"
        ./getresults.sh ${t}6superpeer${j}peer_resptime
    done 

    for i in {1..3}; do
        if [ $i -eq 1 ]; then
            j=1
        elif [ $i -eq 2 ]; then
            j=3
        else
            j=5
        fi
        ./test.sh $t 10 $j
        sleep 120
        echo "getting results for 10 super peers and $j peers per super peer"
        ./getresults.sh ${t}10superpeer${j}peer_resptime
    done 
done

RESULTS_DIR="results/"
RESULTS_FOLDER="results"
col=1

for csv_file in $(find $RESULTS_DIR -name "*.csv"); do
    echo "Processing $csv_file"
    #python3 plot.py $csvf_ile
    avg=$(awk -F ',' '{ sum += $'$col'; n++ } END { if (n > 0) print sum / n }' $csv_file)
    echo "$csv_file,Average response time: $avg" >> $RESULTS_FOLDER/averages.txt
done
