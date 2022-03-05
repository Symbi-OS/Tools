set -x
NAPI=$1
VQ1=$2
VQ2=$3
taskset -c 1 ./poller $NAPI $VQ1 0 0
taskset -c 1 ./poller $NAPI $VQ2 0 0
