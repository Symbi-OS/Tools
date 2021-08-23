#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[]){

  	if(argc != 2){
    	printf("usage ./unroll <app_work>\n");
    	exit(1);
  	}
	int fd = open("write_to.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);

	int work_pwr_2 = atoi(argv[1]);
	int work_total = 1 << work_pwr_2;

	int i = 0; 
    while(i < work_total){
		write(fd,"Hello\n",6);
   		i++; 
	 } 


	close(fd);

    return 0;
}
