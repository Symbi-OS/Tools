CC=gcc
CFLAGS= -g -Wall -Wextra -static #-mno-red-zone

SYMLIB_DIR=../libs/symlib
SYMLIB=$(SYMLIB_DIR)/build/libsym.a
SYMLIB_INCLUDE_DIR=$(SYMLIB_DIR)/include

KALLSYMDIR=../libs/kallsymlib/
KALLSYMLIB=../libs/kallsymlib/libkallsym.a

VM_DISK=../../min-initrd/min-initrd.d/root

DEPENDENCY_LIBS=$(KALLSYMLIB) $(SYMLIB)

# TODO: Add all executables here.
# Stack Starvation DIR
SSD=stack_starvation
TFD=text_fault
I3D=int3_probe
P3D=print
GPD=getppid
POLLD=sym_poll
ECO=echo_server

EXECS=$(SSD)/stack_starvation $(TFD)/text_fault $(I3D)/int3_probe $(P3D)/print_cr3_static $(GPD)/getppid $(POLLD)/poller $(ECO)/client_RW $(ECO)/server_RW

all: $(EXECS)

# TODO: resolve the fact that symlib requires kallsymlib
$(KALLSYMLIB):
	make -C $(KALLSYMDIR)

$(SYMLIB):
	make -C $(SYMLIB_DIR)

$(ECO)/client_RW: $(ECO)/client.c | $(DEPENDENCY_LIBS)
	$(CC) -DUSE_READ_WRITE $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

 # $(ECO)/server_RW

$(ECO)/server_RW: $(ECO)/server.c | $(DEPENDENCY_LIBS)
	gcc -DUSE_READ_WRITE $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(POLLD)/poller: $(POLLD)/poller.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(GPD)/getppid: $(GPD)/getppid.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(P3D)/print_cr3_static: $(P3D)/print_cr3_static.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(SSD)/stack_starvation: $(SSD)/stack_starvation.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR)

$(TFD)/text_fault: $(TFD)/text_fault.c $(TFD)/foo.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

$(I3D)/int3_probe: $(I3D)/int3_probe.c | $(DEPENDENCY_LIBS)
	$(CC) $(CFLAGS) -o $@ $^ $(SYMLIB) $(KALLSYMLIB) -I $(SYMLIB_INCLUDE_DIR) $(KALLSYMLIB)

run_ss:
	taskset -c 0 $(SSD)/stack_starvation

run_tf:
	taskset -c 0 $(TFD)/text_fault

run_i3:
	taskset -c 0 $(I3D)/int3_probe

cp_execs:
	cp_to_disk.sh $(VM_DISK) $(EXECS)

clean:
	rm -rf $(EXECS)

dist_clean: clean
	make -C $(SYMLIB_DIR) clean
	make -C $(KALLSYMDIR) clean

# MY_INCLUDES=-I../../linux/arch/x86/include/generated  -I../../linux/include -I../../linux/arch/x86/include/uapi -I../../linux/arch/x86/include/generated/uapi -I../../linux/include/uapi -I../../linux/include/generated/uapi -I../../linux/include

# write_syscall_loop_static: write_syscall_loop.c ../lib_constructors/elevate.c
# 	$(CC) $(MY_CC_FLAGS) $(MY_STATIC_FLAG) -DSTATIC_BUILD -o $@ $^

# write_syscall_loop_dynamic: write_syscall_loop.c
# 	$(CC) $(MY_CC_FLAGS) -o $@ $< #-I ~/kallsymlib -L ~/kallsymlib -lkallsym


#-I../../linux/arch/x86/include/generated  -I../../linux/include -I../../linux/arch/x86/include/uapi -I../../linux/arch/x86/include/generated/uapi -I../../linux/include/uapi -I../../linux/include/generated/uapi -I../../linux/include



#-I../../linux/include -I../../linux/arch/x86/include

#$(MY_INCLUDES)


# app_user_sys: app_sys_loop.c
# 	gcc -DUSER -DSYSTEM -static $< -o $@

# app_user: app_sys_loop.c
# 	gcc -DUSER -static $< -o $@

# app_sys: app_sys_loop.c
# 	gcc -DSYSTEM -static $< -o $@



# write_syscall_dynamic: write_syscall.c
# 	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors/

# write_syscall_static: write_syscall_static.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

# read_syscall_static: read_syscall_static.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

# while1_symbiote: while1.c ../lib_constructors/elevate.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/

# while1_linux: while1.c
# 	$(CC) $(CC_FLAGS) -o $@ $<

# fork: fork.c ../lib_constructors/elevate.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
# 	cp fork ../app

# fork_linux: fork.c 
# 	$(CC) $(CC_FLAGS) -o $@ $< -DLINUX

# stack_starvation: stack_starvation.c ../include/sym_lib.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ -I ../include/

# getppid: getppid.c  ../lib_constructors/elevate.c
# 	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors

# mmaptest_linux: mmaptest.c
# 	gcc -g -O1 -o $@ $<

# mmaptest_symbiote: mmaptest.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ ../lib_constructors/elevate.c
# # gcc -g -O1 -o $@ $<

# getppid_static: getppid.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

# getppid_static_mt: getppid_mt.c
# 	$(CC) -DPINGPONG -DNOPINNING $(CC_FLAGS) -o $@ $^ -lpthread ../include/sym_lib.c

# getppid_dynamic: getppid_dynamic.c
# 	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^

# steal_syscalls: steal_syscalls.c ../lib_constructors/elevate.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
# 	cp steal_syscalls ../app

# print_cr3_dynamic: print_cr3.c
# 	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors/

# print_cr3_static: print_cr3_static.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

# run:
# 	./write_syscall_static > out.txt
# clean:
# 	- rm fork print_cr3 print_cr3_dynamic stack_starvation steal_syscalls write_syscall write_syscall_static getppid_dynamic  getppid_static print_cr3_static write_syscall_loop_static write_syscall_loop_dynamic

# all: OS_Eval OS_Eval_WO_Elevate
# 	cp OS_Eval ../app
# 	cp OS_Eval_WO_Elevate ../app_no_elevate

# OS_Eval: OS_Eval.c ../lib_constructors/elevate.o
# 	$(CC) $(CC_FLAGS) -o $@ $(LIBS) $^

# OS_Eval_WO_Elevate: OS_Eval.c
# 	$(CC) $(CC_FLAGS) -o $@ $(LIBS) $<

# run: OS_Eval
# 	sudo ./OS_Eval 0 tuOS

# run_no_elevate:
# 	sudo ./OS_Eval_WO_Elevate 0 tuOS

# clean:
# 	-rm OS_Eval OS_Eval_WO_Elevate
# 	-rm -rf output.tuOS.csv
# 	-rm -rf test_file.txt

