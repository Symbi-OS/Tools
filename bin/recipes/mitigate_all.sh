#!/bin/bash

# --all is very important if using isolcpu
NUM_CPUS=$(nproc --all)
for i in $(seq 0 $(($NUM_CPUS - 1)) )
do
    # can bug like this
    # (./interposing_mitigator.sh -m df -t $i && \
    #      ./interposing_mitigator.sh -m tf -t $i ) &
    echo mitigating core $i
    ./interposing_mitigator.sh -m df -t $i
    ./interposing_mitigator.sh -m tf -t $i
done

wait

echo done
