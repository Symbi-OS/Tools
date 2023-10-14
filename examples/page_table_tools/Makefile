CC=gcc
CXX=g++
CFLAGS=-O0 -ggdb -Wall -Wextra -mno-red-zone

SYMLIB_DIR=../../../Symlib
SYMLIB_DYNAM_BUILD_DIR=$(SYMLIB_DIR)/dynam_build
SYMLIB_INCLUDE_DIR=$(SYMLIB_DIR)/include
SYMLIB_LINK=-lSym
CXXFLAGS=$(CFLAGS) -std=c++17

REMOVED_INCLUDE_FLAGS = -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/12/include -I./arch/x86/include -I./arch/x86/include/generated  -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -include ./include/linux/compiler-version.h -include ./include/linux/compiler_types.h -DMODULE  -DKBUILD_BASENAME='"kpt_util"' -DKBUILD_MODNAME='"kpt_util"' -D__KBUILD_MODNAME=kmod_kpt_util -DCC_USING_FENTRY -D__KERNEL__
REMOVED_INCLUDE_FLAGS = 

LINUX_PATH=../../../linux
obj-m += kpt_util.o

all: prep simple_walk readwrite_test page_migration_test snapshot_pid

default_ld_script.ld:
	ld --verbose | awk 'BEGIN {p=0}; /^=======/ {if (p==0) p=1; else p=2; next}; p==1' > $@

prep: default_ld_script.ld

ktest.c: kpt_util.c
	pushd /home/ar/Symbi-OS/linux/ && gcc -Wp,-MMD,/home/ar/Symbi-OS/Tools/examples/page_table_tools/.kpt_util.o.d -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/12/include -I./arch/x86/include -I./arch/x86/include/generated  -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/compiler-version.h -include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h -D__KERNEL__ -fmacro-prefix-map=./= -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Wno-format-security -std=gnu89 -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fcf-protection=none -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-address-of-packed-member -O2 -fno-allow-store-data-races -fno-reorder-blocks -fno-ipa-cp-clone -fno-partial-inlining -Wframe-larger-than=2048 -fstack-protector -Wimplicit-fallthrough=5 -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang -fno-stack-clash-protection -g -gdwarf-4 -pg -mrecord-mcount -mfentry -DCC_USING_FENTRY -Wdeclaration-after-statement -Wvla -Wno-pointer-sign -Wno-stringop-truncation -Wno-zero-length-bounds -Wno-array-bounds -Wno-stringop-overflow -Wno-restrict -Wno-maybe-uninitialized -fno-strict-overflow -fno-stack-check -fconserve-stack -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -Wno-packed-not-aligned  -DMODULE  -DKBUILD_BASENAME='"kpt_util"' -DKBUILD_MODNAME='"kpt_util"' -D__KBUILD_MODNAME=kmod_kpt_util -E -o /home/ar/Symbi-OS/Tools/examples/page_table_tools/ktest.c /home/ar/Symbi-OS/Tools/examples/page_table_tools/kpt_util.c && popd


# We are trying to remove all the directives (includes) that SHOULD HAVE BEEN removed at the preprocessor time
# Removed directives: -I, -include, -nostdinc, -isystem, -D
# [!!!] See $(REMOVED_INCLUDE_FLAGS) at the top of the file
# *Note* The md5sum of the generated file matches exactly what we produced by hand by removing the ftrace_branch_* from the preprocessed file. 
# ***Removed: -mcmodel=kernel
ktest.s: ktest.c
	pushd /home/ar/Symbi-OS/linux/ && gcc -Wp,-MMD,/home/ar/Symbi-OS/Tools/examples/page_table_tools/.kpt_util.o.d $(REMOVED_INCLUDE_FLAGS) -fmacro-prefix-map=./= -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Wno-format-security -std=gnu89 -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fcf-protection=none -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone  -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-address-of-packed-member -O2 -fno-allow-store-data-races -fno-reorder-blocks -fno-ipa-cp-clone -fno-partial-inlining -Wframe-larger-than=2048 -fstack-protector -Wimplicit-fallthrough=5 -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang -fno-stack-clash-protection -g -gdwarf-4 -pg -mrecord-mcount -mfentry -Wdeclaration-after-statement -Wvla -Wno-pointer-sign -Wno-stringop-truncation -Wno-zero-length-bounds -Wno-array-bounds -Wno-stringop-overflow -Wno-restrict -Wno-maybe-uninitialized -fno-strict-overflow -fno-stack-check -fconserve-stack -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -Wno-packed-not-aligned -fPIC -S -o /home/ar/Symbi-OS/Tools/examples/page_table_tools/ktest.s /home/ar/Symbi-OS/Tools/examples/page_table_tools/ktest.c && popd

kpt_util: kpt_util.c
	make -C $(LINUX_PATH) M=$(PWD) modules

kpt_util.o: kpt_util.c
	-make kpt_util
	rm -rf .Module.* .modules.* Module* modules* kpt_util kpt_util.mod* *.ko .kpt_util*

page_table_util.o: page_table_util.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

json11.o: json11.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $^ -o $@

simple_walk.o: simple_walk.c
	$(CC) $(CFLAGS) -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

snapshot_pid.o: snapshot_pid.cpp
	$(CXX) $(CXXFLAGS) -fPIC -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

readwrite_test.o: readwrite_test.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

page_migration_test.o: page_migration_test.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

simple_walk: simple_walk.o page_table_util.o kpt_util.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

simple_walk_with_kallsyms: simple_walk.o page_table_util.o kpt_util.o
	$(CXX) $(CXXFLAGS) -o simple_walk $^ -lSym -L../../../Symlib/dynam_build -L./ -lkallsyms

readwrite_test: readwrite_test.o page_table_util.o kpt_util.o
	$(CC) $(CFLAGS) -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

page_migration_test: page_migration_test.o page_table_util.o kpt_util.o
	$(CC) $(CFLAGS) -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

snapshot_pid: snapshot_pid.o page_table_util.o kpt_util.o json11.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

snapshot_pid.so: snapshot_pid.o page_table_util.o kpt_util.o json11.o
	$(CXX) $(CXXFLAGS) -shared -fPIC -o $@ $^ -lSym -L../../../Symlib/dynam_build -T symhelper.ld

run_simple_walk: simple_walk
	./simple_walk

run_readwrite_test: readwrite_test
	./readwrite_test

run_page_migration_test: page_migration_test
	./page_migration_test

run_snapshot_pid: snapshot_pid
	./snapshot_pid

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
	rm -rf .Module.* .modules.* Module* modules* kpt_util kpt_util.mod* *.ko *.o .kpt_util* simple_walk readwrite_test page_migration_test snapshot_pid snapshot_pid.so ktest.c
