.PHONY = all run clean

all: mylib.so main

mylib.so: mylib.c
	gcc -Wno-int-conversion -I./ -shared -fPIC -o $@ $<

main: main.c
	gcc -I./ -g main.c -o main

run: main mylib.so
	time (LD_PRELOAD=./mylib.so LD_LIBRARY_PATH=$(PWD) ./main 1000000)

perf: main mylib.so
	perf stat env LD_PRELOAD=./mylib.so LD_LIBRARY_PATH=$(PWD) ./main 1000000

clean:
	rm main mylib.so loop.txt
