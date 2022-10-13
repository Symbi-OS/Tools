
all: synch_write

synch_write: synch_write.c
    gcc synch_write.c -o synch_write

run:
    rm write_to.txt
    ./synch_write
clean:
    rm -rf synch_write
