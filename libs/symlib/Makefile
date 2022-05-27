CC=gcc
CFLAGS= -g -D CONFIG_X86_64 -Wall -Wextra #-static -mno-red-zone
.PHONY: clean all

# Ignores TODO: remove them all
# CFLAGS+=-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function

# SRC DIRS
SRC_DIR=src

# SRC FILES
SRC_L0=$(wildcard $(SRC_DIR)/L0/*.c)
SRC_L1=$(wildcard $(SRC_DIR)/L1/*.c)
SRC_L2=$(wildcard $(SRC_DIR)/L2/*.c)
SRC_L3=$(wildcard $(SRC_DIR)/L3/*.c)
SRC_LINF=$(wildcard $(SRC_DIR)/LINF/*.c)
SRC_LIDK=$(wildcard $(SRC_DIR)/LIDK/*.c)

# BUILD DIRECTORIES
BUILD_DIR=build
BUILD_DIR_L0=$(BUILD_DIR)/L0
BUILD_DIR_L1=$(BUILD_DIR)/L1
BUILD_DIR_L2=$(BUILD_DIR)/L2
BUILD_DIR_L3=$(BUILD_DIR)/L3
BUILD_DIR_LINF=$(BUILD_DIR)/LINF
BUILD_DIR_LIDK=$(BUILD_DIR)/LIDK

# HEADERS
HEADER_DIR=include

# Obj list, put them in build dir.
OBJ_L0 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L0))
OBJ_L1 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L1))
OBJ_L2 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L2))
OBJ_L3 = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_L3))
OBJ_LINF = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_LINF))
OBJ_LIDK = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRC_LIDK))

# Objects less than or equal to i.
OBJ_LE0  =$(OBJ_L0)  $(OBJ_LIDK) #NOTE here's IDK, it gets into everything.
OBJ_LE1  =$(OBJ_LE0) $(OBJ_L1)
OBJ_LE2  =$(OBJ_LE1) $(OBJ_L2)
OBJ_LE3  =$(OBJ_LE2) $(OBJ_L3)
OBJ_LEINF=$(OBJ_LE3) $(OBJ_LINF)

# Libs
LIB_L0=$(BUILD_DIR_L0)/L0.a
LIB_L1=$(BUILD_DIR_L1)/L1.a
LIB_L2=$(BUILD_DIR_L2)/L2.a
LIB_L3=$(BUILD_DIR_L3)/L3.a
LIB_LINF=$(BUILD_DIR_LINF)/LINF.a

# The superset lib.
LIB_SYM=$(BUILD_DIR)/libsym.a
ALL_LIBS =$(LIB_L0) $(LIB_L1) $(LIB_L2) $(LIB_L3) $(LIB_LINF) $(LIB_SYM)

boldprint = @printf '\e[1m%s\e[0m\n' $1

all: $(ALL_LIBS)

dynam_L0:
	mkdir -p dynam_build/L0/
	gcc -g -D DYNAM -D CONFIG_X86_64 -Wall -Wextra -fPIC  -c src/L0/sym_lib.c -o dynam_build/L0/sym_lib.o -I include
	gcc -shared -fPIC -o dynam_build/L0/libelevate.so dynam_build/L0/sym_lib.o

# NOTE: Inf and libsym are the same :)
$(LIB_SYM): $(LIB_LINF)
	cp $(LIB_LINF) $@
	$(call boldprint, 'Built libsym.a')

# BUILD LINF lib.
$(LIB_LINF): $(OBJ_LEINF)
	ar rcs $@ $^
	$(call boldprint, 'Built LINF lib')

# BUILD L2 lib.
$(LIB_L3): $(OBJ_LE3)
	ar rcs $@ $^
	$(call boldprint, 'Built L3 lib')

# BUILD L2 lib.
$(LIB_L2): $(OBJ_LE2)
	ar rcs $@ $^
	$(call boldprint, 'Built L2 lib')

# BUILD L1 lib.
$(LIB_L1): $(OBJ_LE1)
	ar rcs $@ $^
	$(call boldprint, 'Built L1 lib')

# BUILD L0 lib.
$(LIB_L0): $(OBJ_LE0)
	ar rcs $@ $^
	$(call boldprint, 'Built L0 lib')

# TODO this shouldn't build all dirs if only building subset of libs...
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(HEADER_DIR)

# XXX what is this trying to do? LIB_LIDK isn't a thing???
# BUILD LINF lib.
$(LIB_LIDK): $(LIB_L0) $(LIB_L1) $(OBJ_L2) $(OBJ_LINF) $(OBJ_LIDK) | $(BUILD_DIR_LIDK)
	ar rcs $@ $(OBJ_LINF) $(OBJ_LIDK) $(OBJ_L2) $(OBJ_L1) $(OBJ_L0)
	@printf '\e[1mBuilt LINF lib\e[0m\n'


clean:
	rm -rf $(BUILD_DIR)
	rm -rf dynam_build
