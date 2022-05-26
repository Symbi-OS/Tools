.PHONY = all run_static app_dynamic clean test

all: libelevate.so #app_static app_dynamic

app_static: elevate.c app_constructor.c
	gcc -c elevate.c -o elevate.o
	ar rcs libelevate.a elevate.o
	gcc -c app_constructor.c -o app_constructor.o
	gcc app_constructor.o elevate.o -o $@
	gcc app_constructor.o -L./ -Wl,--whole-archive -lelevate -Wl,--no-whole-archive -o $@

run_static: app_static
	./app_static

libelevate.so: elevate.c
	gcc -shared -fPIC -o $@ $<
# This is morally equivalent to:
# gcc -c -Wall -Werror -fpic $<
# gcc -shared -o $@ elevate.o


app_dynamic: elevate.c app_constructor.c
	gcc -c -I./ -fPIC -o elevate.pic.o elevate.c
	gcc -shared -o libelevate.so elevate.pic.o
	gcc -I./ -L./ -o $@ app_constructor.c -lelevate

run_dynamic: app_dynamic
	LD_PRELOAD=./libelevate.so LD_LIBRARY_PATH=$(PWD) ./app_dynamic

run_dynamic_preload: app_dynamic
	LD_DEBUG=libs LD_PRELOAD=./libelevate.so LD_LIBRARY_PATH=$(PWD) ./app_dynamic

run_dynamic_no_preload: app_dynamic
	LD_DEBUG=libs LD_LIBRARY_PATH=$(PWD) ./app_dynamic 

# LD_DEBUG=libs LD_LIBRARY_PATH=$(PWD) ./app_dynamic
# LD_PRELOAD=./ LD_DEBUG=libs LD_LIBRARY_PATH=$(PWD) ./app_dynamic

clean:
	-rm *.o *.so *.a app_static app_dynamic libelevate.so

