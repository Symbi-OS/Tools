#!/bin/bash
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)

# Take an argument -d which enables debug mode
if [ "$1" == "-d" ]; then
    set -x
fi

# --all is very important if using isolcpu
NUM_CPUS=$(nproc)
for i in $(seq 0 $((NUM_CPUS - 1))); do
    {
        # Do this in parallel
        (
            # echo mitigating core $i
            # taskset -c $i ${SCRIPT_DIR}/../cr_tool -d
            # if [ $? -ne 0 ]; then
            #     echo "cr_tool failed core $i"
            #     exit 1
            # fi

            "${SCRIPT_DIR}"/interposing_mitigator.sh -m df -t "$i"
            if [ $? -ne 0 ]; then
                echo interposing_mitigator failed for df core "$i"
                exit 1
            fi

            ${SCRIPT_DIR}/interposing_mitigator.sh -m tf -t $i
            if [ $? -ne 0 ]; then
                echo "interposing_mitigator failed for tf core $i"
                exit 1
            fi

            # This one has a weird bug & we use db exceptions now anyway
            # ${SCRIPT_DIR}/interposing_mitigator.sh -m i3 -t $i

            ${SCRIPT_DIR}/interposing_mitigator.sh -m db -t $i
            if [ $? -ne 0 ]; then
                echo "interposing_mitigator failed for db core $i"
                exit 1
            fi
            echo mitigated core "$i"

        ) &
    }
done

wait

echo mitigation finished
