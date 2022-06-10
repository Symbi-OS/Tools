INCLUDE_DIR=../../include/
all: scratchpad_test

CFLAGS=-ggdb -O0 -Wall -Wextra -Werror #-static #-fno-omit-frame-pointer

System.map:
	echo ERROR copy your kernels system map here.

scratchpad_test: scrcatchpad_test.c ../../include/libsym.a ../../include/kallsymlib/libkallsym.a  /boot/System.map-5.14.0-symbiote+

	make -C $(INCLUDE_DIR)
	gcc $(CFLAGS) -o $@ $< ../../include/libsym.a ../../include/kallsymlib/libkallsym.a

run:
	sync
	sync
	sync
	taskset -c 0 ./scratchpad_test

clean:
	rm -rf scratchpad_test *.o
