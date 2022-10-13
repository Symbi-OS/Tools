#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <liburing.h>
#include <string.h>

#define QUEUE_DEPTH 1

int submit_write_request(char* buffer, unsigned length, struct io_uring* ring, int fd, struct io_uring_sqe *sqe){

	struct io_uring_cqe *cqe;

	io_uring_prep_write(sqe, 0, buffer, length, 0);

	sqe->flags |= IOSQE_FIXED_FILE;

	io_uring_submit(ring);

	int ret = io_uring_wait_cqe(ring, &cqe);

	if (ret < 0) {
		fprintf(stderr, "Error waiting for completion: %s\n", strerror(-ret));
        return 1;
    }

	if (cqe->res < 0) {
		fprintf(stderr, "Error in async operation: %s\n", strerror(-cqe->res));
    }

	io_uring_cqe_seen(ring, cqe);

    return 0;
}


int write_io(char* buffer, int length, int fd, struct io_uring ring,  struct io_uring_sqe *sqe) {

	submit_write_request(buffer, length, &ring, fd, sqe);

	return 0;
}

int main(int argc, char *argv[]){

	if(argc != 2){
        printf("usage ./unroll <app_work>\n");
        exit(1);
    }

    int work_pwr_2 = atoi(argv[1]);
    int work_total = 1 << work_pwr_2;

	struct io_uring ring;
	struct io_uring_sqe *sqe;
	struct io_uring_params params;

	memset(&params, 0, sizeof(params));
    params.flags |= IORING_SETUP_SQPOLL;
    params.sq_thread_idle = 10000;

    io_uring_queue_init_params(QUEUE_DEPTH, &ring, &params);

	int fd = open("write_to.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }
	io_uring_register_files(&ring, &fd, 1);

	sqe = io_uring_get_sqe(&ring);

	while(1){
        // app work
        register int work_ctr = work_total;
        while(work_ctr --){
            asm("nop");
        }
		// system work
		write_io("Hello\n", 6, fd, ring, sqe);
    }

	io_uring_queue_exit(&ring);

    close(fd);

	return 0;
}
