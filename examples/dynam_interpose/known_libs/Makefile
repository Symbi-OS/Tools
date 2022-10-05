# Don't forget ld_preload_path

#https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/create-libraries/index
SRC=main.c libA.c libB.c
STATIC_LIBS=libA.a libB.a

all: main_dynamic main_static main_linked_static main_link_dynamically

# all at once compile

# executable is dynamically linked, but "libs" are just compiled in.
main_dynamic: $(SRC)
	gcc $^ -o $@

# all at once compile, build static executable
main_static: $(SRC)
	gcc -static $(SRC) -o $@

# Static executable linked from static libs order matters
main_linked_static: main.o $(STATIC_LIBS)
	gcc -static $^ -o $@

# Notice main.o is not PIC
main_link_dynamically: main.o libA_oneshot.so libB_oneshot.so
	gcc main.o -L./ -lA_oneshot -lB_oneshot -o $@


# one step build, these md5 the same as libA.so and libB.so
libA_oneshot.so: libA.c
	gcc -shared -fPIC -o $@ $<

libB_oneshot.so: libB.c
	gcc -shared -fPIC -o $@ $<


# build shared libs, multistep for clarity.
libA.so: libA.pic.o
	gcc -shared $< -o $@

libB.so: libB.pic.o
	gcc -shared $< -o $@

# build pic objs
libA.pic.o: libA.c
	gcc -c -fPIC $< -o $@

libB.pic.o: libB.c
	gcc -c -fPIC $< -o $@

# build static libs
static: libA.a libB.a

# static libs
libA.a: libA.o
	ar rcs $@ $<

libB.a: libB.o
	ar rcs $@ $<

# objects
main.o: main.c
	gcc -c $< -o $@

libA.o: libA.c libA.h
	gcc -c $< -o $@

libB.o: libB.c libB.h
	gcc -c $< -o $@


clean:
	- rm -rf main_dynamic main_static main_linked_static
	- rm -rf *.o
	- rm -rf *.a
	- rm -rf *.so
	- rm -rf main_link_dynamically
