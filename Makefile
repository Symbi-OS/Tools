GCC_FLAGS=-static -Wfatal-errors -g

app: app.c include/sym_lib.c
	gcc $(GCC_FLAGS) $^ -o $@

clean:
	-rm app
