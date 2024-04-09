# sc_lib.so:
# 	gcc -O2 -ggdb -Wall -Wextra -mno-red-zone -fno-omit-frame-pointer -Wfatal-errors -L ../../../Symlib/dynam_build -I ../../../Symlib/include -shared -fPIC -o sc_lib.so -lSym sc_lib.c sc_lib.h "-UDEEP_SHORTCUT"

# gcc $(CFLAGS) -shared -fPIC -o $@ $(SYMLIB_LINK) $^ $(DEEP_SC_DEFINE)
# Build the .o
# gcc -c -O2 -ggdb -Wall -Wextra -mno-red-zone -fno-omit-frame-pointer -Wfatal-errors -I ../../../Symlib/include "-UDEEP_SHORTCUT" -fPIC -o sc_lib.o sc_lib.c
# Build the .so
CC := gcc
#CFLAGS := -O2 -ggdb -Wall -Wextra -mno-red-zone -fno-omit-frame-pointer -Wfatal-errors
CFLAGS := -O4 -march=native -Wall -Wextra -Wfatal-errors
#-mno-red-zone -fno-omit-frame-pointer -Wfatal-errors

LDFLAGS := -L../../../Symlib/dynam_build -shared -fPIC
LIBS := -lSym
INCLUDE := -I../../../Symlib/include
DEFINE := -UDEEP_SHORTCUT

TARGET := sc_lib.so
SOURCES := sc_lib.c
HEADERS := sc_lib.h

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDE) -o $@ $(LIBS) $(SOURCES) $(DEFINE)

clean:
	rm -rf sc_lib.so sc_lib.o