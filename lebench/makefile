CC=gcc
libs=-pthread -O0
OS_Eval: OS_Eval.c
	$(CC) -o OS_Eval $(libs) OS_Eval.c 

run: OS_Eval
	./OS_Eval 0 tuOS

clean:
	-rm OS_Eval
	-rm output.tuOS.csv
	-rm test_file.txt

