CC=gcc
CXX=g++
CFLAGS=-O0 -ggdb -Wall -Wextra -mno-red-zone -lpthread -lrt

SYMLIB_DIR=../../../Symlib
SYMLIB_DYNAM_BUILD_DIR=$(SYMLIB_DIR)/dynam_build
SYMLIB_INCLUDE_DIR=$(SYMLIB_DIR)/include
SYMLIB_LINK=-lSym
CFLAGS += -D__KERNEL__
CXXFLAGS=$(CFLAGS) -std=c++17

LINUX_PATH=../../../linux
obj-m += kpt_util.o

all: prep simple_walk readwrite_test page_migration_test

default_ld_script.ld:
	ld --verbose | awk 'BEGIN {p=0}; /^=======/ {if (p==0) p=1; else p=2; next}; p==1' > $@

prep: default_ld_script.ld

kpt_util: kpt_util.c
	make -C $(LINUX_PATH) M=$(PWD) modules

kpt_util.o: kpt_util.c
	-make kpt_util
	rm -rf .Module.* .modules.* Module* modules* kpt_util kpt_util.mod* *.ko .kpt_util*

page_table_util.o: page_table_util.c
	$(CC) $(CFLAGS) -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

json11.o: json11.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

simple_walk.o: simple_walk.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

readwrite_test.o: readwrite_test.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

page_migration_test.o: page_migration_test.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

simple_walk: simple_walk.o page_table_util.o kpt_util.o json11.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

readwrite_test: readwrite_test.o page_table_util.o kpt_util.o
	$(CC) $(CFLAGS) -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

page_migration_test: page_migration_test.o page_table_util.o kpt_util.o
	$(CC) $(CFLAGS) -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

run_simple_walk: simple_walk
	./simple_walk

run_readwrite_test: readwrite_test
	./readwrite_test

run_page_migration_test: page_migration_test
	./page_migration_test

run_all_tests: all
	@echo ======== Simple Page Table Walk ========
	./simple_walk

	@echo ======== Read-Write Modification ========
	- ./readwrite_test
	./readwrite_test 1

	@echo ""
	@echo ======== Page Migration Test ========
	./page_migration_test

	@echo ""

clean:
	rm -rf .Module.* .modules.* Module* modules* kpt_util kpt_util.mod* *.ko *.o .kpt_util* simple_walk readwrite_test page_migration_test
