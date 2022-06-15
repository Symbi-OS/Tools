INCLUDE_DIR=../../include/
all: db_probe db_probe_local

CFLAGS=-ggdb -O0 -Wall -Wextra -Werror #-static #-fno-omit-frame-pointer

System.map:
	echo ERROR copy your kernels system map here.

db_probe: db_probe.c ../../include/libsym.a ../../include/kallsymlib/libkallsym.a  /boot/System.map-5.14.0-symbiote+

	make -C $(INCLUDE_DIR)
	gcc $(CFLAGS) -o $@ $< ../../include/libsym.a ../../include/kallsymlib/libkallsym.a

db_probe_local: db_probe_local.c ../../include/libsym.a ../../include/kallsymlib/libkallsym.a  /boot/System.map-5.14.0-symbiote+

	make -C $(INCLUDE_DIR)
	gcc $(CFLAGS) -o $@ $< ../../include/libsym.a ../../include/kallsymlib/libkallsym.a

run:
	sync
	sync
	sync
	taskset -c 0 ./db_probe 0 2

run:
	sync
	sync
	sync
	taskset -c 0 ./db_probe_local

clean:
	rm -rf db_probe db_probe_local *.o
