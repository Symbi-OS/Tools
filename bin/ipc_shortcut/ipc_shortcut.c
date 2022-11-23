#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ipc.h>

static JobRequestBuffer_t* s_JobBuffer = NULL;

#define VERIFY_JOB_BUFFER  if (s_JobBuffer == NULL) { \
        						s_JobBuffer = ipc_get_job_buffer(); \
								if (s_JobBuffer == NULL) { \
									fprintf(stderr, "[-] Failed to retrieve a working job buffer\n"); \
								} \
							}

void __attribute__ ((constructor)) init(void) {
    printf("ipc_shortcut_lib init\n");
}

void __attribute__ ((destructor)) cleanUp(void) {
	if (s_JobBuffer) {
		s_JobBuffer->status = JOB_NO_REQUEST;
	}
}

// int open(const char* path, int flags, ...) {
    // if (s_JobBuffer == NULL) {
    //     s_JobBuffer = ipc_get_job_buffer();
    //     if (s_JobBuffer == NULL) {
    //         fprintf(stderr, "[-] Failed to retrieve a working job buffer\n");
    //     }
    // }

//     s_JobBuffer->cmd = CMD_OPEN;
// 	s_JobBuffer->buffer_len = strlen(path);
// 	memcpy(s_JobBuffer->buffer, path, s_JobBuffer->buffer_len);
// 	s_JobBuffer->arg1 = O_WRONLY | O_APPEND | O_CREAT;

// 	// Indicate that the job was requested
// 	submit_job_request(s_JobBuffer);

// 	// Wait for the job to be completed
// 	wait_for_job_completion(s_JobBuffer);

// 	return s_JobBuffer->response;
// }

// int close(int fd) {
//     s_JobBuffer->cmd = CMD_CLOSE;
// 	s_JobBuffer->arg1 = fd;

// 	// Indicate that the job was requested
// 	submit_job_request(s_JobBuffer);

// 	// Wait for the job to be completed
// 	wait_for_job_completion(s_JobBuffer);

//     return s_JobBuffer->response;
// }

ssize_t write(int fd, const void* buf, size_t len) {
	VERIFY_JOB_BUFFER

	s_JobBuffer->pid = getpid();
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

// ssize_t read(int fd, void* buf, size_t len){
// 	s_JobBuffer->cmd = CMD_READ;
// 	s_JobBuffer->arg1 = fd;
// 	s_JobBuffer->buffer_len = len;

// 	// Indicate that the job was requested
// 	submit_job_request(s_JobBuffer);

// 	// Wait for the job to be completed
// 	wait_for_job_completion(s_JobBuffer);

// 	// Copy the shared memory read buffer to the output buffer
// 	memcpy(buf, s_JobBuffer->buffer, len);

//     return s_JobBuffer->response;
// }
