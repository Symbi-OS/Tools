#!/bin/bash
EL=../examples/elevate_lower/elevate_lower

LAST_CPU=$(( $(nproc ) - 1 ))
NUM_ITERATIONS=$1

for i in $(seq 0 "$LAST_CPU")
do
    echo taskset -c $i $EL $NUM_ITERATIONS
    taskset -c $i $EL $NUM_ITERATIONS &
done

wait

echo at least 1 job finished.
