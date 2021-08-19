CC=gcc
CC_FLAGS=-static -O0 -ggdb -mno-red-zone
CC_FLAGS_DYNAMIC=-O0 -ggdb -mno-red-zone
.PHONY=all

MY_CC_FLAGS=-O0 -ggdb -mno-red-zone #-Wall #-Wfatal-errors
MY_STATIC_FLAG=-static

MY_INCLUDES=-I../../linux/arch/x86/include/generated  -I../../linux/include -I../../linux/arch/x86/include/uapi -I../../linux/arch/x86/include/generated/uapi -I../../linux/include/uapi -I../../linux/include/generated/uapi -I../../linux/include

write_syscall_loop_static: write_syscall_loop.c ../lib_constructors/elevate.c
	$(CC) $(MY_CC_FLAGS) $(MY_STATIC_FLAG) -DSTATIC_BUILD -o $@ $^

write_syscall_loop_dynamic: write_syscall_loop.c
	$(CC) $(MY_CC_FLAGS) -o $@ $< #-I ~/kallsymlib -L ~/kallsymlib -lkallsym


#-I../../linux/arch/x86/include/generated  -I../../linux/include -I../../linux/arch/x86/include/uapi -I../../linux/arch/x86/include/generated/uapi -I../../linux/include/uapi -I../../linux/include/generated/uapi -I../../linux/include



#-I../../linux/include -I../../linux/arch/x86/include

#$(MY_INCLUDES)


app_user_sys: app_sys_loop.c
	gcc -DUSER -DSYSTEM -static $< -o $@

app_user: app_sys_loop.c
	gcc -DUSER -static $< -o $@

app_sys: app_sys_loop.c
	gcc -DSYSTEM -static $< -o $@



# write_syscall_dynamic: write_syscall.c
# 	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors/

# write_syscall_static: write_syscall_static.c
# 	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

read_syscall_static: read_syscall_static.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

while1_symbiote: while1.c ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/

while1_linux: while1.c
	$(CC) $(CC_FLAGS) -o $@ $<

fork: fork.c ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
	cp fork ../app

fork_linux: fork.c 
	$(CC) $(CC_FLAGS) -o $@ $< -DLINUX

stack_starvation: stack_starvation.c ../include/sym_lib.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../include/

# getppid: getppid.c  ../lib_constructors/elevate.c
# 	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors

mmaptest_linux: mmaptest.c
	gcc -g -O1 -o $@ $<

mmaptest_symbiote: mmaptest.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../lib_constructors/elevate.c
# gcc -g -O1 -o $@ $<

getppid_static: getppid.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

getppid_static_mt: getppid_mt.c
	$(CC) -DPINGPONG -DNOPINNING $(CC_FLAGS) -o $@ $^ -lpthread ../include/sym_lib.c

getppid_dynamic: getppid_dynamic.c
	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^

steal_syscalls: steal_syscalls.c ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
	cp steal_syscalls ../app

print_cr3_dynamic: print_cr3.c
	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors/

print_cr3_static: print_cr3_static.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

run:
	./write_syscall_static > out.txt
clean:
	- rm fork print_cr3 print_cr3_dynamic stack_starvation steal_syscalls write_syscall write_syscall_static getppid_dynamic  getppid_static print_cr3_static write_syscall_loop_static write_syscall_loop_dynamic

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

