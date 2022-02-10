/*
	C ECHO client example using sockets
*/
#include <stdio.h>	//printf
#include <string.h>	//strlen
#include <sys/socket.h>	//socket
#include <arpa/inet.h>	//inet_addr
#include <unistd.h>

#include <stdlib.h>

#include <signal.h>
/* #include <stdio.h> */
#include <stdbool.h>
/* #include <unistd.h> */

volatile sig_atomic_t print_flag = false;
#define MSG_SZ 8

void handle_alarm( int sig ) {
  print_flag = true;
}

int main(int argc , char *argv[])
{
	int sock;
	struct sockaddr_in server;
	char server_reply[MSG_SZ];
	char client_message[MSG_SZ];
  client_message[0] = 'h';
  client_message[1] = 'i';
  client_message[2] = '!';
  client_message[3] = (char) 0;

  unsigned long runs = 1UL << 13;

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	/* server.sin_addr.s_addr = inet_addr("1.1.1.2"); */
	/* server.sin_addr.s_addr = inet_addr("127.0.0.1"); */
  if(argc !=2){
    printf("expects ./client_RW <ip addr server>\n");
    exit(-1);
  }
	server.sin_addr.s_addr = inet_addr(argv[1]);
	server.sin_family = AF_INET;
	server.sin_port = htons( 8888 );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}

	puts("Connected\n");

	//keep communicating with server
	int count = 0;

	while(runs--)
	{
		//printf("Enter message : ");
		//scanf("%s" , message);

		//Send some data
#ifdef USE_SEND_RECV
		/* if( send(sock , "hi!" , strlen("hi!") , 0) < 0) */
		if( send(sock , client_message, MSG_SZ , 0) < 0)
#endif
#ifdef USE_READ_WRITE
		if( write(sock , client_message, MSG_SZ) < 0)
#endif
		{
			puts("Send failed");
			return 1;
		}

		//Receive a reply from the server

#ifdef USE_SEND_RECV
		if( recv(sock , server_reply , MSG_SZ , 0) < 0)
#endif
#ifdef USE_READ_WRITE
		if( read(sock , server_reply , MSG_SZ) < 0)
#endif
		{
			puts("recv failed");
			break;
		}

		//puts("Server reply :");
		//puts(server_reply);
		/* if(++count % 10000 == 0) */
		/* 	printf("%d\n",count++); */
	}

	close(sock);
	return 0;
}
