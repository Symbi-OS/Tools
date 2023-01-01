#include "common.h"
#include "ipc.h"
#include <sys/syscall.h>
#include <errno.h>
#include <pthread.h>

#define FD_PER_CLIENT 10000

int target_logfd = -1;
static uint8_t bShouldExit = 0;
pthread_spinlock_t locks[MAX_JOB_BUFFERS];

static int registered_fds[MAX_JOB_BUFFERS][FD_PER_CLIENT] = {};

void* workspace_thread(void* ws){
	workspace_t *workspace = (workspace_t *) ws;
	register int idx = 0;
	JobRequestBuffer_t* job_buffer;

	while (!bShouldExit) {
		// obtain lock to update next buffer
        while (1) {
			if (pthread_spin_trylock(&locks[idx]) == 0){
				if (workspace->job_buffers[idx].status == JOB_REQUESTED){
					goto JOB_FOUND;
				}else{
					pthread_spin_unlock(&locks[idx]);
					idx++;
					if (idx==MAX_JOB_BUFFERS){
						idx = 0;
					}
				}
			}else{
				idx++;
				if (idx==MAX_JOB_BUFFERS){
					idx = 0;
				}
			}
        }
		
		JOB_FOUND:
		job_buffer = &workspace->job_buffers[idx];
        // Process the requested command
		//printf("request %d received.\n", job_buffer->cmd);
		switch (job_buffer->cmd) {
		case CMD_WRITE: {
			//printf("write request received\n");
			int clientfd = job_buffer->arg1;
			
			if (registered_fds[idx][clientfd] == 0) {
				int pidfd = syscall(SYS_pidfd_open, job_buffer->pid, 0);
				int borrowed_fd = syscall(SYS_pidfd_getfd, pidfd, job_buffer->arg1, 0);

				// check error case
				if (borrowed_fd == -1) {
					perror("pidfd_getfd");
					//  print errno
					printf("errno: %d\n", errno);
					//  print strerror
					printf("strerror: %s\n", strerror(errno));
					break;
				}

				registered_fds[idx][clientfd] = borrowed_fd;
			}
			
			int server_fd = registered_fds[idx][clientfd];

			//printf("Client fd: %d, Server fd: %d\n", clientfd, server_fd);

			job_buffer->response = write(server_fd, job_buffer->buffer, job_buffer->buffer_len);
			
			// Write to borrowed_fd
			if (job_buffer->response == -1) {
				perror("write failed");
			}

			break;
		}
		case CMD_READ: {
			int clientfd = job_buffer->arg1;

            if (registered_fds[idx][clientfd] == 0) {
                int pidfd = syscall(SYS_pidfd_open, job_buffer->pid, 0);
                int borrowed_fd = syscall(SYS_pidfd_getfd, pidfd, job_buffer->arg1, 0);

                // check error case
                if (borrowed_fd == -1) {
                    perror("pidfd_getfd");
                    //  print errno
                    printf("errno: %d\n", errno);
                    //  print strerror
                    printf("strerror: %s\n", strerror(errno));
                    break;
                }

                registered_fds[idx][clientfd] = borrowed_fd;
            }

            int server_fd = registered_fds[idx][clientfd];

			job_buffer->response = read(server_fd, job_buffer->buffer, job_buffer->buffer_len);

            // Write to borrowed_fd
            if (job_buffer->response == -1) {
                perror("read failed");
            }
			break;
		}
		case CMD_CLOSE: {
			//printf("close request received\n");
			int clientfd = job_buffer->arg1;
			registered_fds[idx][clientfd] = 0;
			//print_job_buffer(job_buffer);
			break;
		}
		case CMD_KILL_SERVER: {
			bShouldExit = 1;
			break;
		}
		case CMD_DISCONNECT: {
			memset(&registered_fds[idx], 0, sizeof(registered_fds[0]));
			//printf("disconnecting from the server\n");
			break;
		}
		default: break;
		}

        // Updating the job status flag
		//printf("Request completed\n");
		//printf("sleeping...\n");
		//sleep(2);
		//printf("sleeping done\n");
		mark_job_completed(job_buffer);
		pthread_spin_unlock(&locks[idx]);
	}

	pthread_exit(NULL);
}



int main(int argc, char** argv) {

	int num_threads = 1;

	if (argc < 1){
		printf("Usage: ./<server binary> [optional]<nthreads>\n");
	} else if (argc == 2) {
		num_threads = atoi(argv[1]);
	}

	workspace_t* workspace = ipc_connect_server();

	if (workspace == NULL){
        printf("Fail to intialize server...\n");
    }
	
	pthread_t tid[num_threads];

	for (int j = 0; j < MAX_JOB_BUFFERS; j++){
		if (pthread_spin_init(&(locks[j]), PTHREAD_PROCESS_PRIVATE) != 0) {
			printf("\n mutex init has failed\n");
			return 1;
		}
	}

	for (int j = 0; j < num_threads; j++){
		int err = pthread_create(&(tid[j]), NULL, &workspace_thread, workspace);
		printf("[IPC Server] Thread starting at: %p\n", &tid[j]);
		if (err != 0) printf("can't create thread :[%s]", strerror(err));
	}

	for (int j = 0; j < num_threads; j++){
		pthread_join(tid[j], NULL);
	}

	ipc_close();
	return 0;
}
