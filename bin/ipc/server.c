#include "common.h"
#include "ipc.h"
#include <sys/syscall.h>
#include <errno.h>
#include <pthread.h>
#include <sys/resource.h>

#define FD_PER_CLIENT 10000

int target_logfd = -1;
static uint8_t bShouldExit = 0;

static int registered_fds[MAX_JOB_BUFFERS][FD_PER_CLIENT] = {};


void* workspace_thread(void* ws){
	workspace_t *workspace = (workspace_t *) ws;
	register int idx = 0;
	JobRequestBuffer_t* job_buffer;

	while (!bShouldExit) {
		// obtain lock to update next buffer
        	idx = pick_up_job(ws);
		
		job_buffer = &workspace->job_buffers[idx];

        // Process the requested command
		switch (job_buffer->cmd) {
		case CMD_WRITE: {
			int clientfd = job_buffer->arg1;
			
			if (registered_fds[idx][clientfd] == 0) {
				int pidfd = syscall(SYS_pidfd_open, job_buffer->pid, 0);
				int borrowed_fd = syscall(SYS_pidfd_getfd, pidfd, job_buffer->arg1, 0);

				// check error case
				if (borrowed_fd == -1) {
					perror("pidfd_getfd");
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
			int clientfd = job_buffer->arg1;
			close(registered_fds[idx][clientfd]);
			registered_fds[idx][clientfd] = 0;
			break;
		}
		case CMD_KILL_SERVER: {
			bShouldExit = 1;
			break;
		}
		case CMD_DISCONNECT: {
			memset(&registered_fds[idx], 0, sizeof(registered_fds[0]));
			break;
		}
		default: break;
		}

		mark_job_completed(job_buffer);
	}

	pthread_exit(NULL);
}



int main(int argc, char** argv) {
	int num_threads = 1;

	// By default, every process has a soft and a hard limit on how many
	// opened handles it can have, and the soft limit is usually around 1024.
	// We bypass this limitation by setting the soft limit to the hard limit.
	struct rlimit resource_limit;
  
    // Get old limits
    if (getrlimit(RLIMIT_NOFILE, &resource_limit) != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	// Set new value
    resource_limit.rlim_cur = resource_limit.rlim_max;
  
    // Set new limits
    if(setrlimit(RLIMIT_NOFILE, &resource_limit) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
		return -1;
	}

	if (argc < 1) {
		printf("Usage: ./<server binary> [optional]<nthreads>\n");
	} else if (argc == 2) {
		num_threads = atoi(argv[1]);
	}

	workspace_t* workspace = ipc_connect_server();

	if (workspace == NULL){
        printf("Fail to intialize server...\n");
    }
	
	pthread_t tid[num_threads];

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
