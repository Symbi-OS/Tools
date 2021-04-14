CC=gcc
CC_FLAGS=-static -O0 -ggdb -mno-red-zone
CC_FLAGS_DYNAMIC=-O0 -ggdb -mno-red-zone
.PHONY=all
write_syscall: write_syscall.c
	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors/

fork: fork.c ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
	cp fork ../app

stack_starvation: stack_starvation.c ../include/sym_lib.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../include/

getppid: getppid.c  ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors

steal_syscalls: steal_syscalls.c ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
	cp steal_syscalls ../app

print_cr3_dynamic: print_cr3.c
	$(CC) $(CC_FLAGS_DYNAMIC) -o $@ $^ -I ../lib_constructors/

print_cr3_static: print_cr3_static.c
	$(CC) $(CC_FLAGS) -o $@ $^ ../include/sym_lib.c

clean:
	rm fork print_cr3 print_cr3_dynamic stack_starvation steal_syscalls write_syscall getppid print_cr3_static 

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

