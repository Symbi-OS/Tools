.PHONY = all run_static app_dynamic clean test

all: app_static app_dynamic

app_static: elevate.c app_constructor.c
	gcc -c elevate.c -o elevate.o
	ar rcs libelevate.a elevate.o
	gcc -c app_constructor.c -o app_constructor.o
	gcc app_constructor.o -L./ -lelevate -o $@

run_static: app_static
	./app_static

app_dynamic: elevate.c app_constructor.c
	gcc -c -I./ -fPIC -o elevate.pic.o elevate.c
	gcc -shared -o libelevate.so elevate.pic.o
	gcc -I./ -L./ -o $@ app_constructor.c -lelevate

run_dynamic: app_dynamic
	LD_LIBRARY_PATH=$(PWD) ./app_dynamic

clean:
	-rm *.o *.so *.a app_static app_dynamic

