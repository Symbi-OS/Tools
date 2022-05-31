#!/bin/bash
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# --all is very important if using isolcpu
NUM_CPUS=$(nproc --all)
for i in $(seq 0 $(($NUM_CPUS - 1)) )
do
    # can bug like this
    # (./interposing_mitigator.sh -m df -t $i && \
    #      ./interposing_mitigator.sh -m tf -t $i ) &
    {
    echo mitigating core $i
    ${SCRIPT_DIR}/interposing_mitigator.sh -m df -t $i
    ${SCRIPT_DIR}/interposing_mitigator.sh -m tf -t $i
    ${SCRIPT_DIR}/interposing_mitigator.sh -m i3 -t $i
    echo done mitigation core $i
    }&
done

wait

echo done
