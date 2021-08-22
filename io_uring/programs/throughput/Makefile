LIBS=-l uring

all: reg io io_poll batch_io batch_io_poll

reg: reg.c
	gcc reg.c -o reg

io: io.c
	gcc $< -o $@ $(LIBS)

batch_io: batch_io.c
	gcc $< -o $@ $(LIBS)

io_poll: io_poll.c
	gcc $< -o $@ $(LIBS)

batch_io_poll: batch_io_poll.c
	gcc $< -o $@ $(LIBS)
