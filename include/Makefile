SRC = $(wildcard *.c)               # list of source files
OBJS = $(patsubst %.c, %.o, $(SRC)) # list of object files
CFLAGS =-mno-red-zone -g -D CONFIG_X86_64 -Wall

all: libsym.a #libelevate.so libelevate_cons.so

objs : $(OBJS)                      # target
	- rm -rf ./build
	mkdir ./build
	mv *.o build

# build/arch_x86.o: ./arch_x86.S objs
# 	gcc $(CFLAGS) -c -o $@ $<

libsym.a: objs #build/arch_x86.o
	ar rcs $@ ./build/*.o

libelevate.so: sym_interrupts.c sym_lib_syscall.c
	gcc -shared -fPIC -o $@ $^

libelevate_cons.so: sym_interrupts.c sym_lib_syscall.c sym_constructors.c
	gcc -shared -fPIC -o $@ $^


clean:
	-rm -rf *.so *.o *.a ./build
