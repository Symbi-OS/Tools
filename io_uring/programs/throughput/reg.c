#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[]){

  	if(argc != 3){
    	printf("usage ./unroll <app_work>\n");
    	exit(1);
  	}
	int fd = open("write_to.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);

	int work_pwr_2 = atoi(argv[1]);
	int iter_number = atoi(argv[2]);
	int work_total = 1 << work_pwr_2;

	int i = 0; 
	clock_t start = clock();
    while(i < work_total){
		write(fd,"Hello\n",6);
   		i++; 
	 } 

	clock_t end = clock();

    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    FILE *fptr = fopen("stats/throughput/reg.txt", "a");

    fprintf(fptr,"%d,%d,%f\n",  work_pwr_2, iter_number, cpu_time_used);

    fclose(fptr);

	close(fd);

    return 0;
}
