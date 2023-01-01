#include "ipc.h"
#include "stdlib.h"

int main(int argc, char** argv) {
	(void)argc;
	int threads = atoi(argv[1]);

    for (int i=0; i < threads; i++){
        JobRequestBuffer_t* job_buffer = ipc_get_job_buffer();
        if (!job_buffer) {
            return -1;
        }

        job_buffer->cmd = CMD_KILL_SERVER;
        submit_job_request(job_buffer);
        wait_for_job_completion(job_buffer);
    }
    

    return 0;
}