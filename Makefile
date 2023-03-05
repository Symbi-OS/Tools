SHELL := /bin/bash

CC=gcc
# XXX: We do see this break mitigate when set to -O3
CFLAGS= -O0 -ggdb -Wall -Wextra -mno-red-zone -fno-omit-frame-pointer
CFLAGS+=-Wfatal-errors -Wfatal-errors
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
