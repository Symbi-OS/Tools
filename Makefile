# Use bash
SHELL := /bin/bash
# If building with gcc

# XXX: We do see this break mitigate when set to -O3

# Common compiler flags
# -O0, -ggdb: Debugging and no optimization
# -Wall, -Wextra: Show all warnings
# -Wfatal-errors: Stop build on first error
# -mno-red-zone, -fno-omit-frame-pointer: For debugging and avoiding red zone problems before mitigation.
CFLAGS= -O0 -ggdb -Wall -Wextra -Wfatal-errors -mno-red-zone -fno-omit-frame-pointer 

# Compiler selection
# If CLANG=true, use clang; otherwise, use gcc
ifeq ($(CLANG),true)
	CC=clang
	MAKE=scan-build make
# do scan-build
else
	CC=gcc
	CFLAGS+=-fanalyzer
endif

export CC CFLAGS

SYMLIB_DIR=../../Symlib
SYMLIB_DYNAM_BUILD_DIR=$(SYMLIB_DIR)/dynam_build
SYMLIB_STATIC=$(SYMLIB_DIR)/build/libsym.a
SYMLIB_INCLUDE_DIR=$(SYMLIB_DIR)/include
SYMLIB_LINK=-lSym

ifeq ($(TYPE), static)
	SYMLIB_LINK=$(SYMLIB_STATIC)
endif

export SYMLIB_DIR SYMLIB_STATIC SYMLIB_INCLUDE_DIR SYMLIB_DYNAM_BUILD_DIR SYMLIB_LINK

LIBRARY_PATH=$(SYMLIB_DYNAM_BUILD_DIR)
export LIBRARY_PATH

# TODO: why does this print white then red lol?
boldprint = @printf '\e[1m%s\e[0m\n' $1

# NOTE: Can flip to 1 for debugging.
CPUS=$(shell nproc)

.PHONY: all bin examples clean_bin clean_examples

all: bin #examples

install_compiler_tools:
	dnf install gcc clang clang-analyzer cppcheck

cppcheck:
	cppcheck .

mitigate:
	./bin/recipes/mitigate_all.sh

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
