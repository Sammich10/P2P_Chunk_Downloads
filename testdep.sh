if [ $# -ne 3 ]; then
  echo "Usage: test.sh <type(a2a|tree)> <num super peers> <num peers per super peer>"
  exit 1
fi
if [ $1 != "a2a" ] && [ $1 != "tree" ]; then
  echo "Usage: test.sh <type(a2a|tree)> <num super peers> <num peers per super peer>"
  exit 1
fi

if [ $2 -lt 1 ] || [ $2 -gt 10 ]; then
  echo "Number of super peers must be between 1 and 10"
  exit 1
fi

if [ $3 -lt 1 ] || [ $3 -gt 5 ]; then
  echo "Number of peers per super peer must be between 1 and 5"
  exit 1
fi

./init.sh $1 $2 $3

echo "Starting ${2} super peers"

for i in $(seq 1 $2); do
  ./superpeer.o peer_configs/super_peers/superpeer${i}.cfg > logs/superpeer${i}.log 2>&1 &
done

sleep $2

j=$(($2 * $3))

echo "Starting ${j} peers in host only mode"

for x in $(seq 2 $j); do
  ./peernode.o peer_configs/weak_peers/peer${x}.cfg -h > logs/peer${x}.log 2>&1 &
done

ps

echo "Starting peer 1 in interactive mode"

./peernode.o peer_configs/weak_peers/peer1.cfg -i
