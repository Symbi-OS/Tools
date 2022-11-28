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
