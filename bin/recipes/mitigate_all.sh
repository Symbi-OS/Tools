#!/bin/bash

NUM_CPUS=$(nproc)

for i in $(seq 0 $NUM_CPUS)
do
	  ./interposing_mitigator.sh -m df -t $i -d
	  ./interposing_mitigator.sh -m tf -t $i -d
done
