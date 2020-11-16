CC=gcc
LIBS=-pthread -O0
CC_FLAGS=-static
.PHONY=all

all: OS_Eval OS_Eval_WO_Elevate
	cp OS_Eval ../app
	cp OS_Eval_WO_Elevate ../app_no_elevate

OS_Eval: OS_Eval.c ../lib_constructors/elevate.o
	$(CC) $(CC_FLAGS) -o $@ $(LIBS) $^

OS_Eval_WO_Elevate: OS_Eval.c
	$(CC) $(CC_FLAGS) -o $@ $(LIBS) $<

run: OS_Eval
	sudo ./OS_Eval 0 tuOS

run_no_elevate:
	sudo ./OS_Eval_WO_Elevate 0 tuOS

clean:
	-rm OS_Eval OS_Eval_WO_Elevate
	-rm -rf output.tuOS.csv
	-rm -rf test_file.txt

