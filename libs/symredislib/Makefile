INCLUDE_DIR=../../include/

all: sym_shortcuts.o

CC=gcc
CFLAGS=-g -D CONFIG_X86_64 -Wall -Wextra -mno-red-zone

sym_shortcuts.o: sym_shortcuts.c sym_shortcuts.h
	gcc $(CFLAGS) sym_shortcuts.c -c -I ../symlib/include

clean:
	rm -f sym_shortcuts.o
