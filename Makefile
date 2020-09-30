GCC_FLAGS=-static -Wfatal-errors

app: app.c
	gcc $(GCC_FLAGS) $< -o $@

clean:
	-rm app
