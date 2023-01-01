#include "common.h"
#include "ipc.h"

#define LOG_FILE "tmp.log"

int64_t calc_average(int64_t* buffer, uint64_t count) {
	int64_t sum = 0;
	for (uint64_t i = 0; i < count; i++) {
		sum += buffer[i];
	}
	return sum / count;
}

void stress_test(int iterations) {
	int logfd = open(LOG_FILE, O_WRONLY | O_CREAT, 0666);

	// Create performance timers
	struct timespec outerTimeStart={0,0}, outerTimeEnd={0,0},
					innerTimeStart={0,0}, innerTimeEnd={0,0};

	// Buffer to hold inner loop times
	int64_t* innerTimesInNs = (int64_t*)malloc(iterations * sizeof(int64_t));

	// Start outer performance timer
    clock_gettime(CLOCK_MONOTONIC, &outerTimeStart);

	for (int i = 0; i < iterations; ++i) {
		// Start inner performance timer
    	clock_gettime(CLOCK_MONOTONIC, &innerTimeStart);

		(void) !write(logfd, "ksys_write\r", 11);

		register int count = 20000;
		while (count) {
			asm volatile ("nop");
			count --;
		}
		// Stop the inner performance timer
    	clock_gettime(CLOCK_MONOTONIC, &innerTimeEnd);

		innerTimesInNs[i] = innerTimeEnd.tv_nsec - innerTimeStart.tv_nsec;
		if (innerTimesInNs[i] < 0 && i > 0) {
			innerTimesInNs[i] = innerTimesInNs[i - 1];
		}
	}

	// Stop the outer performance timer
    clock_gettime(CLOCK_MONOTONIC, &outerTimeEnd);

	// Close the log file handle
	close(logfd);	

	// Print the results
	double cpu_time_used = ((double)outerTimeEnd.tv_sec + 1.0e-9*outerTimeEnd.tv_nsec) -
						   ((double)outerTimeStart.tv_sec + 1.0e-9*outerTimeStart.tv_nsec);
  	fprintf(stderr, "%f\n", cpu_time_used);
	printf("Time used: %f seconds\n", cpu_time_used);
	printf("Throughput: %d req per second\n", (int)(iterations/cpu_time_used));
	// Calculate average latency for each iteration
	//int64_t avgIterationLatency = calc_average(innerTimesInNs, iterations);
	//fprintf(stderr, "%ld\n", avgIterationLatency);

	// Release the timer buffer
	free(innerTimesInNs);
}

int main(int argc, char** argv) {
	(void)argc;
	int iterations = atoi(argv[1]);

	// Run the stress test
	stress_test(iterations);

	return 0;
}
