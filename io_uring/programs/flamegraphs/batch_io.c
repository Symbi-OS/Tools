#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <liburing.h>
#include <string.h>

#define BUF_SIZE    512
#define FILE_NAME1  "write_to.txt"
#define STR1        "Hello\n"

int main(int argc, char *argv[]) {
  	if(argc != 2){
        printf("usage ./unroll <app_work>\n");
        exit(1);
    }

    int work_pwr_2 = atoi(argv[1]);
    int work_total = 1 << work_pwr_2;

	struct io_uring ring;

	int queue_depth;
	if(work_pwr_2 < 9){
		queue_depth = work_total;
	}
	else {
		queue_depth = 1 << 9; 
	}

	int ret = io_uring_queue_init(queue_depth, &ring, 0);
    if (ret) {
        fprintf(stderr, "Unable to setup io_uring: %s\n", strerror(-ret));
        return 1;
    }
//    start_sq_polling_ops(&ring);

	int fd;
    char buff1[BUF_SIZE];
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    int str1_sz = strlen(STR1);

    fd = open(FILE_NAME1, O_RDWR | O_APPEND | O_CREAT, 0644);
    if (fd < 0 ) {
        perror("open");
        return 1;
    }

    memset(buff1, 0, BUF_SIZE);
    strncpy(buff1, STR1, str1_sz);

	clock_t start = clock(); 
	if(work_pwr_2 < 9){
    	for(int i = 0; i < work_total; i++){
 			sqe = io_uring_get_sqe(&ring);
    		if (!sqe) {
        		fprintf(stderr, "Could not get SQE.\n");
        		return 1;
    		}
    		io_uring_prep_write(sqe, fd, buff1, str1_sz, 0);

			void* point = (void*) 7; 
    
    		io_uring_sqe_set_data(sqe, point);

    	}

    	io_uring_submit(&ring);

    	for(int i = 0; i < work_total; i ++) {
        	int ret = io_uring_wait_cqe(&ring, &cqe);
        	if (ret < 0) {
            	fprintf(stderr, "Error waiting for completion: %s\n",
                    strerror(-ret));
            	return 1;
        	}
        	/* Now that we have the CQE, let's process the data */
        	if (cqe->res < 0) {
            	fprintf(stderr, "Error in async operation: %s\n", strerror(-cqe->res));
        	}
//        	printf("Result of the operation: %d\n", cqe->res);
        	io_uring_cqe_seen(&ring, cqe);
    	}
	}
	else{
	int loop_time = work_total/(1<<9);

	int inner_loop_time = 1<<9; 
	for(int j = 0; j < loop_time; j++){

		for(int i = 0; i < inner_loop_time; i++){
            sqe = io_uring_get_sqe(&ring);
        	if (!sqe) {
            	fprintf(stderr, "Could not get SQE.\n");
            	return 1;
        	}
			io_uring_prep_write(sqe, fd, buff1, str1_sz, 0);

            void* point = (void*) 7; 
    
            io_uring_sqe_set_data(sqe, point);

        }

        io_uring_submit(&ring);

        for(int i = 0; i < inner_loop_time; i ++) {
            int ret = io_uring_wait_cqe(&ring, &cqe);
            if (ret < 0) {
                fprintf(stderr, "Error waiting for completion: %s\n",
                    strerror(-ret));
                return 1;
            }
            /* Now that we have the CQE, let's process the data */
            if (cqe->res < 0) {
                fprintf(stderr, "Error in async operation: %s\n", strerror(-cqe->res));
            }
//          printf("Result of the operation: %d\n", cqe->res);
            io_uring_cqe_seen(&ring, cqe);

		}
	}

	}

    io_uring_queue_exit(&ring);
	close(fd);
    return 0;
	
}
