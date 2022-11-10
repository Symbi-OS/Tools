#!/bin/bash

EX=../examples/
SHORTCUT=../bin/shortcut/shortcut
SCD=shortcut_data
SVD=$EX/static_v_dynam
DYNAM=$SVD/test_dynam
STATIC=$SVD/test_static
SVD=$EX/static_v_dynam
SVDS=$SVD/sym
DYNAM_SYM=$SVDS/test_dynam
STATIC_SYM=$SVDS/test_static
TASKSET="taskset -c 0"
SVDD=static_v_dynam_data
SVDSD=static_v_dynam_sym_data
INTD=interpose_data

echo "LD_LIBRARY_PATH: $LD_LIBRARY_PATH"

mkdir $SVDD
for i in {1..20}
do
	echo "EXECUTING RUN: $i"
	RUN=$SVDD/$i
	mkdir $RUN
	sudo perf stat $TASKSET sh -c "./$DYNAM" &> $RUN/perf_dynam.data
	sudo perf stat $TASKSET sh -c "./$STATIC" &> $RUN/perf_static.data	
	sudo perf stat -e power/energy-cores/ $TASKSET sh -c "./$DYNAM" &> $RUN/power_cores_dynam.data	
	sudo perf stat -e power/energy-cores/ $TASKSET sh -c "./$STATIC" &> $RUN/power_cores_static.data	
	sudo perf stat -e power/energy-ram/ $TASKSET sh -c "./$DYNAM" &> $RUN/power_mem_dynam.data
	sudo perf stat -e power/energy-ram/ $TASKSET sh -c "./$STATIC" &> $RUN/power_mem_static.data
done

mkdir $SVDSD
for i in {1..20}
do
	echo "EXECUTING RUN: $i"
	RUN=$SVDSD/$i
	mkdir $RUN
	sudo perf stat $TASKSET sh -c "LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./$DYNAM_SYM" &> $RUN/perf_dynam.data
	sudo perf stat $TASKSET sh -c "./$STATIC_SYM" &> $RUN/perf_static.data	
	sudo perf stat -e power/energy-cores/ $TASKSET sh -c "LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./$DYNAM_SYM" &> $RUN/power_cores_dynam.data	
	sudo perf stat -e power/energy-cores/ $TASKSET sh -c "./$STATIC_SYM" &> $RUN/power_cores_static.data	
	sudo perf stat -e power/energy-ram/ $TASKSET sh -c "LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./$DYNAM_SYM" &> $RUN/power_mem_dynam.data
	sudo perf stat -e power/energy-ram/ $TASKSET sh -c "./$STATIC_SYM" &> $RUN/power_mem_static.data
done

mkdir $SCD
for i in {1..20}
do
	echo "EXECUTING RUN: $i"
	RUN=$SCD/$i
	mkdir $RUN
	sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH make perf_shortcut &> $RUN/perf_dynam.data
	sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH make power_cores_shortcut &> $RUN/power_cores_dynam.data	
	sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH make power_mem_shortcut &> $RUN/power_mem_dynam.data
done

mkdir $INTD
for i in {1..20}
do
	echo "EXECUTING RUN: $i"
	RUN=$INTD/$i
	mkdir $RUN
	sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH make perf_interpose &> $RUN/perf_dynam.data
	sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH make power_cores_interpose &> $RUN/power_cores_dynam.data	
	sudo LD_LIBRARY_PATH=$LD_LIBRARY_PATH make power_mem_interpose &> $RUN/power_mem_dynam.data
done

