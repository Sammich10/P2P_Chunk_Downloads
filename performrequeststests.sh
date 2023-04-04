
#perform tests on A2A overlay network with 1, 2, 4, 8, 16 peers requesting
for i in {1..50}; do
    ./test2.sh a2a 10 4 1
    sleep 15
    ./getresults.sh a2a_1_peer_requsting
done

for i in {1..20}; do
    ./test2.sh a2a 10 4 2
    sleep 25
    ./getresults.sh a2a_2_peer_requsting
done

for i in {1..5}; do
    ./test2.sh a2a 10 4 4
    sleep 35
    ./getresults.sh a2a_4_peer_requsting
done

for i in {1..3}; do
    ./test2.sh a2a 10 4 8
    sleep 45
    ./getresults.sh a2a_8_peer_requsting
done

for i in {1..2}; do
    ./test2.sh a2a 10 4 16
    sleep 60
    ./getresults.sh a2a_16_peer_requsting
done


#perform tests on tree overlay network with 1, 2, 4, 8, 16 peers requesting
for i in {1..50}; do
    ./test2.sh  tree 10 4 1
    sleep 15
    ./getresults.sh  tree_1_peer_requsting
done

for i in {1..20}; do
    ./test2.sh  tree 10 4 2
    sleep 25
    ./getresults.sh  tree_2_peer_requsting
done

for i in {1..5}; do
    ./test2.sh  tree 10 4 4
    sleep 35
    ./getresults.sh  tree_4_peer_requsting
done

for i in {1..3}; do
    ./test2.sh  tree 10 4 8
    sleep 45
    ./getresults.sh  tree_8_peer_requsting
done

for i in {1..2}; do
    ./test2.sh  tree 10 4 16
    sleep 60
    ./getresults.sh  tree_16_peer_requsting
done

#get all the averages
RESULTS_DIR="results/"
RESULTS_FOLDER="results"
col=1

for csv_file in $(find $RESULTS_DIR -name "*.csv"); do
    echo "Processing $csv_file"
    #python3 plot.py $csvf_ile
    avg=$(awk -F ',' '{ sum += $'$col'; n++ } END { if (n > 0) print sum / n }' $csv_file)
    echo "0{$csv_file%.*}, $avg" >> $RESULTS_FOLDER/averages.txt
done
