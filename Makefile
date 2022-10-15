CC=gcc
CFLAGS= -O0 -ggdb -Wall -Wextra -mno-red-zone
export CC CFLAGS


SYMLIB_DIR=../../Symlib
SYMLIB_DYNAM_BUILD_DIR=$(SYMLIB_DIR)/dynam_build
SYMLIB=$(SYMLIB_DYNAM_BUILD_DIR)/libSym.so
SYMLIB_INCLUDE_DIR=$(SYMLIB_DIR)/include
export SYMLIB_DIR SYMLIB SYMLIB_INCLUDE_DIR SYMLIB_DYNAM_BUILD_DIR

LIBRARY_PATH=$(SYMLIB_DYNAM_BUILD_DIR)
LD_LIBRARY_PATH=$(SYMLIB_DYNAM_BUILD_DIR)
export LIBRARY_PATH LD_LIBRARY_PATH

KALLSYMDIR=../../kallsymlib/
KALLSYMLIB=../../kallsymlib/libkallsym.a

DEPENDENCY_LIBS=$(KALLSYMLIB) $(SYMLIB)
export KALLSYMDIR KALLSYMLIB DEPENDENCY_LIBS

# TODO: why does this print white then red lol?
boldprint = @printf '\e[1m%s\e[0m\n' $1

# NOTE: Can flip to 1 for debugging.
CPUS=$(shell nproc)

.PHONY: all bin examples clean_bin clean_examples

all: bin examples

bin:
	$(call boldprint, 'Make bin')
	cd bin && $(MAKE) all -j$(CPUS)

examples:
	$(call boldprint, 'Make examples')
	cd examples && $(MAKE) all -j$(CPUS)

examples_test:
	$(call boldprint, 'Make examples')
	cd examples && $(MAKE) test -j$(CPUS)

clean: clean_bin clean_examples

clean_bin:
	$(call boldprint, 'Clean bin')
	cd bin && $(MAKE) clean

clean_examples:
	$(call boldprint, 'Clean examples')
	cd examples && $(MAKE) clean
