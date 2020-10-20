GCC_FLAGS=-static -Wfatal-errors -g

app: app.c
	gcc $(GCC_FLAGS) $< -o $@

clean:
	-rm app
