LIBS=-l uring

all: asynch_polling_write

asynch_polling_write: asynch_polling_write.c
	gcc $< -o $@ $(LIBS)

run:
	rm write_to.txt
	sudo ./asynch_polling_write
clean:
	rm -rf asynch_polling_write
