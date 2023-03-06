rm logs/*

for i in {1..16}
do
    if [ ! -d "peer_files/p${i}_files" ]
    then
        mkdir "peer_files/p${i}_files"
    fi
    
    rm -r peer_files/p${i}_files
done

pkill -f ./peernode.o
pkill -f ./server.o