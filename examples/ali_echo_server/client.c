#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#define PORT 5555    /* the port client will be connecting to */
#define MAXDATASIZE 100 /* max number of bytes we can get at once */
int main(int argc, char *argv[])
{
    int runs = 5000, i = 0;
    int num = 0;
    int sockfd, numbytes;
    char sendbuf[MAXDATASIZE];
    char buf[MAXDATASIZE];
    struct hostent *he;
    struct sockaddr_in their_addr; /* connector's address information */
    struct timeval t1, t2;
    double elapsedTime;
    double average = 0;
    double totalTime = 0;
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    if ((he=gethostbyname(argv[1])) == NULL) {  /* get the host info */
        herror("gethostbyname");
        exit(1);
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    their_addr.sin_family = AF_INET;      /* host byte order */
    their_addr.sin_port = htons(PORT);    /* short, network byte order */
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    bzero(&(their_addr.sin_zero), 8);     /* zero the rest of the struct */
    if (connect(sockfd, (struct sockaddr *)&their_addr, \
                                          sizeof(struct sockaddr)) == -1) {
        perror("connect");
        exit(1);
    }

    for (i = 0; i < runs; i++){
            sprintf(sendbuf, "%d\n", num);
            gettimeofday(&t1, NULL);
            if (send(sockfd, sendbuf, MAXDATASIZE, 0) == -1){
                    perror("send");
                    exit (1);
            }
            num++;
            if ((numbytes=recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
                    perror("recv");
                    exit(1);
            }
            gettimeofday(&t2, NULL);
            /* printf("Received text=: %s \n", buf); */
            elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
            elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
            printf("elapsedTime = %f ms\n", elapsedTime);
            /* buf[numbytes] = '\0'; */
    }
    close(sockfd);
    return 0;
}
