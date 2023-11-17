CC=gcc
CC_SYM=/home/sym/Symbi-OS/gcc/build/gcc/xgcc -B/home/sym/Symbi-OS/gcc/build/gcc/

CFLAGS=-O0 -ggdb -Wall -Wextra -mno-red-zone -fPIC

SYMLIB_DIR=../../../Symlib
SYMLIB_DYNAM_BUILD_DIR=$(SYMLIB_DIR)/dynam_build
SYMLIB_INCLUDE_DIR=$(SYMLIB_DIR)/include
SYMLIB_LINK=-lSym

REMOVED_INCLUDE_FLAGS = -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/12/include -I./arch/x86/include -I./arch/x86/include/generated  -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -include ./include/linux/compiler-version.h -include ./include/linux/compiler_types.h -DMODULE  -DKBUILD_BASENAME='"kmod"' -DKBUILD_MODNAME='"kmod"' -D__KBUILD_MODNAME=kmod_kmod -DCC_USING_FENTRY -D__KERNEL__
REMOVED_INCLUDE_FLAGS = 

LINUX_PATH=../../../linux
obj-m += kmod.o

all: main

default_ld_script.ld:
	ld --verbose | awk 'BEGIN {p=0}; /^=======/ {if (p==0) p=1; else p=2; next}; p==1' > $@

prep: default_ld_script.ld

kmod_preprocessed.c: kmod.c
	pushd /home/sym/Symbi-OS/linux/ && $(CC_SYM) -Wp,-MMD,/home/sym/Symbi-OS/Tools/examples/kmod_subsystem/.kmod.o.d -nostdinc -isystem /usr/lib/gcc/x86_64-redhat-linux/13/include -I./arch/x86/include -I./arch/x86/include/generated  -I./include -I./arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/compiler-version.h -include ./include/linux/kconfig.h -include ./include/linux/compiler_types.h -D__KERNEL__ -fmacro-prefix-map=./= -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Wno-format-security -std=gnu89 -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fcf-protection=none -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone -mcmodel=kernel -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-address-of-packed-member -O2 -fno-allow-store-data-races -fno-reorder-blocks -fno-ipa-cp-clone -fno-partial-inlining -Wframe-larger-than=2048 -fstack-protector -Wimplicit-fallthrough=5 -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang -fno-stack-clash-protection -g -gdwarf-4 -pg -mrecord-mcount -mfentry -DCC_USING_FENTRY -Wdeclaration-after-statement -Wvla -Wno-pointer-sign -Wno-stringop-truncation -Wno-zero-length-bounds -Wno-array-bounds -Wno-stringop-overflow -Wno-restrict -Wno-maybe-uninitialized -fno-strict-overflow -fno-stack-check -fconserve-stack -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -Wno-packed-not-aligned  -DMODULE  -DKBUILD_BASENAME='"kmod"' -DKBUILD_MODNAME='"kmod"' -D__KBUILD_MODNAME=kmod_kmod -E -o /home/sym/Symbi-OS/Tools/examples/kmod_subsystem/kmod_preprocessed.c /home/sym/Symbi-OS/Tools/examples/kmod_subsystem/kmod.c && popd


# We are trying to remove all the directives (includes) that SHOULD HAVE BEEN removed at the preprocessor time
# Removed directives: -I, -include, -nostdinc, -isystem, -D
# [!!!] See $(REMOVED_INCLUDE_FLAGS) at the top of the file
# *Note* The md5sum of the generated file matches exactly what we produced by hand by removing the ftrace_branch_* from the preprocessed file. 
# ***Removed: -mcmodel=kernel
kmod.s: kmod_preprocessed.c
	pushd /home/sym/Symbi-OS/linux/ && $(CC_SYM) -Wp,-MMD,/home/sym/Symbi-OS/Tools/examples/kmod_subsystem/.kmod.o.d $(REMOVED_INCLUDE_FLAGS) -fmacro-prefix-map=./= -Wall -Wundef -Werror=strict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -fshort-wchar -fno-PIE -Werror=implicit-function-declaration -Werror=implicit-int -Werror=return-type -Wno-format-security -std=gnu89 -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -mno-avx -fcf-protection=none -m64 -falign-jumps=1 -falign-loops=1 -mno-80387 -mno-fp-ret-in-387 -mpreferred-stack-boundary=3 -mskip-rax-setup -mtune=generic -mno-red-zone  -Wno-sign-compare -fno-asynchronous-unwind-tables -mindirect-branch=thunk-extern -mindirect-branch-register -fno-jump-tables -fno-delete-null-pointer-checks -Wno-frame-address -Wno-format-truncation -Wno-format-overflow -Wno-address-of-packed-member -O2 -fno-allow-store-data-races -fno-reorder-blocks -fno-ipa-cp-clone -fno-partial-inlining -Wframe-larger-than=2048 -fstack-protector -Wimplicit-fallthrough=5 -Wno-unused-but-set-variable -Wno-unused-const-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -ftrivial-auto-var-init=zero -enable-trivial-auto-var-init-zero-knowing-it-will-be-removed-from-clang -fno-stack-clash-protection -g -gdwarf-4 -pg -mrecord-mcount -mfentry -Wdeclaration-after-statement -Wvla -Wno-pointer-sign -Wno-stringop-truncation -Wno-zero-length-bounds -Wno-array-bounds -Wno-stringop-overflow -Wno-restrict -Wno-maybe-uninitialized -fno-strict-overflow -fno-stack-check -fconserve-stack -Werror=date-time -Werror=incompatible-pointer-types -Werror=designated-init -Wno-packed-not-aligned -fPIC -S -o /home/sym/Symbi-OS/Tools/examples/kmod_subsystem/kmod.s /home/sym/Symbi-OS/Tools/examples/kmod_subsystem/kmod_preprocessed.c && popd

kmod_fpic.o: kmod.s
	as $^ -o $@

kmod: kmod.c
	make -C $(LINUX_PATH) M=$(PWD) modules

kmod.o: kmod.c
	-make kmod
	rm -rf .Module.* .modules.* Module* modules* kmod kmod.mod* *.ko .kmod*

main.o: main.c
	$(CC) $(CFLAGS) -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

main: main.o kmod.o
	$(CC) $(CFLAGS) -o $@ $^ -lSym -L$(SYMLIB_DYNAM_BUILD_DIR) -T symhelper.ld

libmain.so: main.o kmod_fpic.o
	$(CC) $(CFLAGS) -shared -fPIC -o $@ $^ -lSym -L$(SYMLIB_DYNAM_BUILD_DIR) -T symhelper.ld

test_libmain.o: test_libmain.c
	$(CC) $(CFLAGS) -c $^ -o $@ -I$(SYMLIB_INCLUDE_DIR)

test_libmain: test_libmain.o libmain.so
	$(CC) $(CFLAGS) -o $@ $^ -lSym -L$(SYMLIB_DYNAM_BUILD_DIR) libmain.so -L.. -T symhelper.ld

clean:
	rm -rf .Module.* .modules.* Module* modules* kmod kmod.mod* *.ko *.o .kmod* kmod.s simple_walk readwrite_test page_migration_test snapshot_pid snapshot_pid.so kmod_preprocessed.c test_libmain *.so
