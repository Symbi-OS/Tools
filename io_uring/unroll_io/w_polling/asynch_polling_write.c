//(src) https://unixism.net/loti/tutorial/sq_poll.html
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <liburing.h>
#include <string.h>

int start_sq_polling_ops(struct io_uring *ring, char buff1[], int str1_sz, int fds[]) {

	//io_uring_prep_write(sqe, fd, buffer, length, 0);
	struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    sqe = io_uring_get_sqe(ring);
    if (!sqe) {
        fprintf(stderr, "Could not get SQE.\n");
        return 1;
    }

	off_t offset = 10; 
    io_uring_prep_write(sqe, 0, buff1, str1_sz, offset);
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

}


int main(int argc, char *argv[]) {
    
	if(argc != 2){
        printf("usage ./unroll <app_work>\n");
        exit(1);
    }

	int work_pwr_2 = atoi(argv[1]);
    int work_total = 1 << work_pwr_2;

	struct io_uring ring;
    struct io_uring_params params;

    if (geteuid()) {
        fprintf(stderr, "You need root privileges to run this program.\n");
        return 1;
    }

    memset(&params, 0, sizeof(params));
    params.flags |= IORING_SETUP_SQPOLL;
    params.sq_thread_idle = 2000;

    int ret = io_uring_queue_init_params(8, &ring, &params);
    if (ret) {
        fprintf(stderr, "Unable to setup io_uring: %s\n", strerror(-ret));
        return 1;
    }


	int fds[1];
	char* buff1 = "Hello\n";
	int str1_sz = 6; 
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;

    fds[0] = open("write_to.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fds[0] < 0 ) {
        perror("open");
        return 1;
    }

    ret = io_uring_register_files(&ring, fds, 1);
    if(ret) {
        fprintf(stderr, "Error registering buffers: %s", strerror(-ret));
        return 1;
    }

	//int i = 0;

	//int loop_duration = 1 << work_total; 

//	while(i < loop_duration){

	while(1){
		// app work
		register int work_ctr = work_total; 
		while(work_ctr --){
			asm("nop");
		}
		// system work
		start_sq_polling_ops(&ring,buff1, str1_sz, fds);
		io_uring_queue_exit(&ring);
		ret = io_uring_queue_init_params(8, &ring, &params);
    	if (ret) { 
			fprintf(stderr, "Unable to setup io_uring: %s\n", strerror(-ret));
			return 1;
    	}
		ret = io_uring_register_files(&ring, fds, 1);
    	if(ret) {
			fprintf(stderr, "Error registering buffers: %s", strerror(-ret));
        	return 1;
    	}

//		i++; 
	} 
	return 0;
}

