CC=gcc
CC_FLAGS=-static -O0 -ggdb -mno-red-zone
CC_FLAGS_DYNAMIC=-O0 -ggdb -mno-red-zone
.PHONY=all

MY_CC_FLAGS=-O0 -ggdb -mno-red-zone #-Wall #-Wfatal-errors
MY_STATIC_FLAG=-static

MY_INCLUDES=-I../../linux/arch/x86/include/generated  -I../../linux/include -I../../linux/arch/x86/include/uapi -I../../linux/arch/x86/include/generated/uapi -I../../linux/include/uapi -I../../linux/include/generated/uapi -I../../linux/include

sym_sys: sym_sys.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../../include/sym_lib_syscall.c

run:
	taskset -c 0 ./sym_sys

cp_to_disk: ./sym_sys
# create tmp dir
	mkdir ./mt
	sudo mount -o loop=/dev/loop0 ~tommyunger/ukl/min-initrd/min-initrd.d/root ./mt
	sudo cp sym_sys ./mt
	sleep 1
	sudo umount ./mt
	rm -d ./mt

clean:
	-rm sym_sys
