NAPI=$1
NUM=$2
SLEEP=$3
for (( i = 0; i<$NUM; i++)); do
	taskset -c 1 ./poller $NAPI 0 0 2 &> /dev/null
	sleep $SLEEP
done
