#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ipc.h"

static JobRequestBuffer_t* s_JobBuffer = NULL;

void __attribute__ ((constructor)) init(void) {
    printf("[sym] ipc_shortcut_lib init\n");
    s_JobBuffer = ipc_get_job_buffer();
    if (s_JobBuffer == NULL) {
        fprintf(stderr, "[-] Failed to retrieve a working job buffer\n");
		exit(1);
    }
	s_JobBuffer->pid = getpid();
}

void __attribute__ ((destructor)) cleanUp(void) {
	printf("[sym] disconecting from the server\n");
	if (!s_JobBuffer) {
		fprintf(stderr, "[-] Failed to retrieve a working job buffer\n");
		exit(1);
	}
	disconnect_job_buffer(s_JobBuffer);
}

ssize_t write(int fd, const void* buf, size_t len) {
	if (s_JobBuffer == NULL) {
        fprintf(stderr, "[IPC Server] Failed to retrieve a working job buffer\n");
		exit(1);
    }

    s_JobBuffer->cmd = CMD_WRITE;
	s_JobBuffer->arg1 = fd;
    memcpy(s_JobBuffer->buffer, buf, len);
    s_JobBuffer->buffer_len = len;

	// Indicate that the job was requested
	submit_job_request(s_JobBuffer);

	// Wait for the job to be completed
	wait_for_job_completion(s_JobBuffer);

    return s_JobBuffer->response;
}

ssize_t read(int fd, void* buf, size_t len) {
	if (s_JobBuffer == NULL) {
        fprintf(stderr, "[IPC Server] Failed to retrieve a working job buffer\n");
        exit(1);
    }

	s_JobBuffer->cmd = CMD_READ;
    s_JobBuffer->arg1 = fd;
    s_JobBuffer->buffer_len = len;

    // Indicate that the job was requested
    submit_job_request(s_JobBuffer);

    // Wait for the job to be completed
    wait_for_job_completion(s_JobBuffer);

	memcpy(buf, s_JobBuffer->buffer, len);
    return s_JobBuffer->response;
}

int close(int fd) {
    s_JobBuffer->cmd = CMD_CLOSE;
	s_JobBuffer->arg1 = fd;

	// Indicate that the job was requested
	submit_job_request(s_JobBuffer);

	// Wait for the job to be completed
	wait_for_job_completion(s_JobBuffer);

    return s_JobBuffer->response;
}
