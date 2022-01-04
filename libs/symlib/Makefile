# CFLAGS=-mno-red-zone -g -D CONFIG_X86_64 -Wall
CC=gcc

# SRC DIRS
SRC_DIR=src
SRC_DIR_L0=$(SRC_DIR)/L0
SRC_DIR_L1=$(SRC_DIR)/L1
# SRC_DIR_L2=$(SRC_DIR)/L2

# SRC FILES
SRC_L0=$(wildcard $(SRC_DIR_L0)/*.c)
# SRC_L1=$(wildcard $(SRC_DIR_L1)/*.c)

# BUILD DIRECTORIES
BUILD_DIR=build
BUILD_DIR_L0=$(BUILD_DIR)/L0
BUILD_DIR_L1=$(BUILD_DIR)/L1

# HEADERS
HEADER_DIR=include

# Obj list, put them in build dir.
OBJ_L0 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L0))
OBJ_L1 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L1))

# Libs
L0_LIB=$(BUILD_DIR_L0)/L0.a
L1_LIB=$(BUILD_DIR_L1)/L1.a


# BUILD L1 lib.
# $(BUILD_DIR_L1)/L1.a: $(L1_LIB) $(OBJ_L1) | $(BUILD_DIR_L1)
# ar rcs $@ ./build/L1/*.o #./build/L0/*.o


# BUILD L0 lib.
$(L0_LIB): $(OBJ_L0) | $(BUILD_DIR_L0)
	ar rcs $@ ./build/L0/*.o
	@printf '\e[1mBuilt L0 lib\e[0m\n'

$(BUILD_DIR_L0)/%.o: $(SRC_DIR_L0)/%.c $(BUILD_DIR_L0)
	gcc $(CFLAGS) -c $< -o $@ -I$(HEADER_DIR)

$(BUILD_DIR_L1): $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR_L0): $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR):
	mkdir $@

# obj/%.o : %.c
# 	@echo $<
# 	gcc -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

# libsym.a: objs #build/arch_x86.o
# 	ar rcs $@ ./build/*.o

# libelevate.so: sym_interrupts.c sym_lib_syscall.c
# 	gcc -shared -fPIC -o $@ $^

# libelevate_cons.so: sym_interrupts.c sym_lib_syscall.c sym_constructors.c
# 	gcc -shared -fPIC -o $@ $^


# clean:
# 	-rm -rf *.so *.o *.a ./build
