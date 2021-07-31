#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <liburing.h>

#define QUEUE_DEPTH 1

int get_completion_and_print(struct io_uring *ring, struct io_uring_cqe *cqe){

	int ret = io_uring_wait_cqe(ring, &cqe);

	if (ret < 0) {
        perror("io_uring_wait_cqe");
        return 1;
    }

	return 0; 	
}


int submit_write_request(char* buffer, unsigned length, struct io_uring* ring, int fd,  struct io_uring_sqe *sqe){

	io_uring_prep_write(sqe, fd, buffer, length, 0);

	void* point = (void*) 7; 
	
	io_uring_sqe_set_data(sqe, point);

	io_uring_submit(ring);

    return 0;
}


int write_io(char* buffer, int length, int fd, struct io_uring ring, struct io_uring_cqe *cqe,  struct io_uring_sqe *sqe) {

	submit_write_request(buffer, length, &ring, fd, sqe);

	get_completion_and_print(&ring, cqe); 

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

	struct io_uring_cqe *cqe;

    io_uring_queue_init(QUEUE_DEPTH, &ring, 0); 

	struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);

	int fd = open("write_to.txt", O_WRONLY | O_APPEND | O_CREAT,  0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

	while(1){
	// app work
        register int work_ctr = work_total;
        while (work_ctr--) {
            asm("nop");
        }
	// system work
    	write_io("Hello\n", 6, fd, ring, cqe, sqe);
	}
	io_uring_queue_exit(&ring);

	close(fd);

    return 0; 
}
