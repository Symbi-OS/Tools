#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include "LINF/sym_all.h"

typedef int(*ksys_write_t)(unsigned int fd, const char *buf, size_t count);

int set_affinity(int cpu) {
	cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask); // set the affinity of the process to CPU 1

    return sched_setaffinity(0, sizeof(mask), &mask);
}

int get_affinity() {
	cpu_set_t mask;
    CPU_ZERO(&mask);

    if (sched_getaffinity(0, sizeof(mask), &mask) == -1) {
        perror("sched_getaffinity");
		return -1;
    } else {
        for (int i = 0; i < CPU_SETSIZE; i++) {
            if (CPU_ISSET(i, &mask)) {
                return i;
            }
        }
    }

	return -1;
}

void print_gs_reg_value() {
	unsigned long gs_value;
    asm volatile("rdmsr" : "=a"(gs_value) : "c"(0xC0000102));
    fprintf(stderr, "gs_reg: 0x%lx\n", gs_value);
}

uint64_t get_gsbase() {
	uint64_t gs;
	asm("rdgsbase %0" : "=rm"(gs) : : "memory" );
	return gs;
}

void print_user_and_kern_gsbase() {
	fprintf(stderr, "user_gsbase: 0x%lx\n", get_gsbase());
    asm("swapgs");
    uint64_t kern_gs = get_gsbase();
    asm("swapgs");
    fprintf(stderr, "kern_gsbase: 0x%lx\n", kern_gs);	
}

void do_5_second_loop() {
	clock_t start = clock();
    double duration;

    while (1) {
        duration = (clock() - start) / (double) CLOCKS_PER_SEC;
        if (duration >= 5.0) {
            break;
        }
    }
}

int main(int argc, char** argv) {
	int target_cpu_core = 2;
	if (argc > 1) {
		target_cpu_core = atoi(argv[1]);
	}

	int current_cpu = get_affinity();
	printf("Current CPU Affinity: Core #%i\n", current_cpu);
	
	do_5_second_loop();

	fprintf(stderr, "user_gsbase: 0x%lx\n", get_gsbase());

	ksys_write_t ksys_write = (ksys_write_t)sym_get_fn_address("ksys_write");

	sym_elevate();
	ksys_write(2, "Successfully elevated!\n", 23);

	do_5_second_loop();

	print_gs_reg_value();
	print_user_and_kern_gsbase();

	if (set_affinity(target_cpu_core) == -1) {
		perror("sched_affinity failed");
		return -1;
	}

	current_cpu = get_affinity();
	printf("Current CPU Affinity: Core #%i\n", current_cpu);

	// moving kernel gsbase register value into user gsbase register
	asm("swapgs"); // switch to kernel gs
	uint64_t kernel_gs = get_gsbase();
	asm("swapgs"); // switch back to user gs

	asm("wrgsbase %0" : : "r"(kernel_gs) : );

	print_gs_reg_value();
	print_user_and_kern_gsbase();

	do_5_second_loop();

	printf("Exiting\n");
	return 0;
}

