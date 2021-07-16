#include <fcntl.h>
#include <stdio.h>
#include <liburing.h>
#include <stdlib.h>

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


int submit_write_request(char* filepath, char* buffer, unsigned length, struct io_uring* ring){

	int fd = open(filepath, O_WRONLY | O_APPEND | O_CREAT, 0644);

	if (fd < 0) {
        perror("open");
        return 1;
    }

	struct io_uring_sqe *sqe = io_uring_get_sqe(ring);

	io_uring_prep_write(sqe, fd, buffer, length, 0);

	void* point = (void*) 7; 
	
	io_uring_sqe_set_data(sqe, point);

	io_uring_submit(ring);


    return 0;

}

int main(int argc, char *argv[]) {
    struct io_uring ring;

	char* filepath = argv[1];

	char* buffer = argv[2]; 

	unsigned length = (unsigned) atoi(argv[3]); 

	io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

	int ret = submit_write_request(filepath, buffer, length, &ring);

	get_completion_and_print(&ring); 

	io_uring_queue_exit(&ring);

	return 0;

}
