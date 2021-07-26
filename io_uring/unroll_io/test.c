#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <liburing.h>
#include <string.h>


#define QUEUE_DEPTH 1

int get_completion_and_print(struct io_uring *ring){
	struct io_uring_cqe *cqe;

	int ret = io_uring_wait_cqe(ring, &cqe);

	if (ret < 0) {
        perror("io_uring_wait_cqe");
        return 1;
    }


//	printf("got user data %p \n", io_uring_cqe_get_data(cqe)); 


	return 0; 	
}


int submit_write_request(char* buffer, unsigned length, struct io_uring* ring, int fd){

	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	if (!sqe) {
        fprintf(stderr, "Could not get SQE.\n");
        return 1;
    }	

//sqe->flags |= IOSQE_FIXED_FILE;

	io_uring_prep_write(sqe, fd, buffer, length, 0);

	void* point = (void*) 7; 
	
	io_uring_sqe_set_data(sqe, point);

	io_uring_submit(ring);


    return 0;

}

int write_io(char* buffer, int length, int fd) {

	struct io_uring ring;

	struct io_uring_params params;

	memset(&params, 0, sizeof(params));
    params.flags |= IORING_SETUP_SQPOLL;
    params.sq_thread_idle = 2000;


	int ret = io_uring_queue_init_params(QUEUE_DEPTH, &ring, &params);

	if (ret) {
        fprintf(stderr, "Unable to setup io_uring: %s\n", strerror(-ret));
        return 1;
    }	

	ret = submit_write_request(buffer, length, &ring, fd);

	get_completion_and_print(&ring); 

	io_uring_queue_exit(&ring);

	return 0;

}




int main(int argc, char *argv[]){

	int fd = open("write_to.txt", O_WRONLY | O_APPEND | O_CREAT,  0644);


//	int fd = open("write_to.txt", O_DIRECT); 

    if (fd < 0) {
        perror("open");
        return 1;
    }

	write_io("Hello", 6, fd);

	close(fd); 

	
    return 0; 
}
