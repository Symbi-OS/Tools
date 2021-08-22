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

	io_uring_cqe_seen(ring, cqe);

	return 0; 	
}



int write_io(char* buffer, int length, int fd, struct io_uring ring, struct io_uring_cqe *cqe,  struct io_uring_sqe *sqe) {

	io_uring_prep_write(sqe, 0, buffer, length, 0);

    sqe->flags |= IOSQE_FIXED_FILE;

    io_uring_submit(&ring);

	get_completion_and_print(&ring, cqe); 

	return 0;
}

int main(int argc, char *argv[]){

	if(argc != 3){
        printf("usage ./unroll <app_work>\n");
        exit(1);
    }

    int work_pwr_2 = atoi(argv[1]);
	int iter_number = atoi(argv[2]);
    int work_total = 1 << work_pwr_2;

	struct io_uring ring;

	struct io_uring_cqe *cqe;

    io_uring_queue_init(QUEUE_DEPTH, &ring, IORING_SETUP_SQPOLL); 

	struct io_uring_sqe *sqe = io_uring_get_sqe(&ring);

	int fd = open("write_to.txt", O_WRONLY | O_APPEND | O_CREAT,  0644);
    if (fd < 0) {
        perror("open");
        return 1;
    }

	io_uring_register_files(&ring, &fd, 1);

	clock_t start = clock();

	for(int i = 0; i < work_total; i++){
		write_io("Hello\n", 6, fd, ring, cqe, sqe);
	}

	clock_t end = clock();

    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    FILE *fptr = fopen("stats/throughput/io_poll.txt", "a");

	fprintf(fptr,"%d,%d,%f\n",  work_pwr_2, iter_number, cpu_time_used);

    fclose(fptr);

	io_uring_queue_exit(&ring);

	close(fd);

    return 0; 
}
