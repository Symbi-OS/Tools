CC=gcc
CFLAGS= -g -D CONFIG_X86_64 -Wall -Wextra #-static -mno-red-zone

# Ignores TODO: remove them all
# CFLAGS+=-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function

# SRC DIRS
SRC_DIR=src

# SRC FILES
SRC_L0=$(wildcard $(SRC_DIR)/L0/*.c)
SRC_L1=$(wildcard $(SRC_DIR)/L1/*.c)
SRC_L2=$(wildcard $(SRC_DIR)/L2/*.c)

# BUILD DIRECTORIES
BUILD_DIR=build
BUILD_DIR_L0=$(BUILD_DIR)/L0
BUILD_DIR_L1=$(BUILD_DIR)/L1
BUILD_DIR_L2=$(BUILD_DIR)/L2

# HEADERS
HEADER_DIR=include

# Obj list, put them in build dir.
OBJ_L0 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L0))
OBJ_L1 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L1))
OBJ_L2 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L2))

# Libs
LIB_L0=$(BUILD_DIR_L0)/L0.a
LIB_L1=$(BUILD_DIR_L1)/L1.a
LIB_L2=$(BUILD_DIR_L2)/L2.a

# The superset lib.
LIB_SYM=$(BUILD_DIR)/libsym.a

$(LIB_SYM): $(LIB_L0) $(LIB_L1) $(LIB_L2)
	ar rcs $@ $(OBJ_L0) $(OBJ_L1) $(OBJ_L2)
	@printf '\e[1mBuilt libsym.a\e[0m\n'

# BUILD L2 lib.
$(LIB_L2): $(LIB_L0) $(LIB_L1) $(OBJ_L2) | $(BUILD_DIR_L2)
	ar rcs $@ $(OBJ_L2) $(OBJ_L1) $(OBJ_L0)
	@printf '\e[1mBuilt L2 lib\e[0m\n'

# BUILD L1 lib.
$(LIB_L1): $(LIB_L0) $(OBJ_L1) | $(BUILD_DIR_L1)
	ar rcs $@ $(OBJ_L1) $(OBJ_L0)
	@printf '\e[1mBuilt L1 lib\e[0m\n'

# BUILD L0 lib.
$(LIB_L0): $(OBJ_L0) | $(BUILD_DIR_L0)
	ar rcs $@ $(OBJ_L0)
	@printf '\e[1mBuilt L0 lib\e[0m\n'

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR_L0) $(BUILD_DIR_L1) $(BUILD_DIR_L2)
	gcc $(CFLAGS) -c $< -o $@ -I $(HEADER_DIR)

$(BUILD_DIR_L2): $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR_L1): $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR_L0): $(BUILD_DIR)
	mkdir $@

$(BUILD_DIR):
	mkdir $@

clean:
	rm -rf $(BUILD_DIR)


# libelevate.so: sym_interrupts.c sym_lib_syscall.c
# 	gcc -shared -fPIC -o $@ $^

# libelevate_cons.so: sym_interrupts.c sym_lib_syscall.c sym_constructors.c
# 	gcc -shared -fPIC -o $@ $^


# clean:
# 	-rm -rf *.so *.o *.a ./build
