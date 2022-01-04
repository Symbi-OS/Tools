INCLUDE_DIR=../../include/
all: int3_probe

CFLAGS=-ggdb -O0 -Wall -Wextra -Werror #-static #-fno-omit-frame-pointer

System.map:
	echo ERROR copy your kernels system map here.

int3_probe: int3_probe.c ../../include/libsym.a ../../include/kallsymlib/libkallsym.a  /boot/System.map-5.14.0-symbiote+
	make -C $(INCLUDE_DIR)
	gcc $(CFLAGS) -o $@ $< ../../include/libsym.a ../../include/kallsymlib/libkallsym.a

run:
	sync
	sync
	sync
	taskset -c 0 ./int3_probe

clean:
	- rm -rf int3_probe *.o
