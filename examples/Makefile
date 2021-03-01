CC=gcc
CC_FLAGS=-static -O0 -ggdb -mno-red-zone
.PHONY=all

fork: fork.c ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
	cp fork ../app

steal_syscalls: steal_syscalls.c ../lib_constructors/elevate.c
	$(CC) $(CC_FLAGS) -o $@ $^ -I ../lib_constructors/
	cp steal_syscalls ../app

clean:
	rm fork

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

