LIBS=-l uring

all: asynch_write

asynch_write: asynch_write.c
    gcc $< -o $@ $(LIBS)

run:
    rm write_to.txt
    ./asynch_write
clean:
    rm -rf asynch_write


