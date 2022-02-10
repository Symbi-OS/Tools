/*
	C socket server example
*/

#include<stdio.h>
#include<string.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write

#define MSG_SZ 8
int use_shortcut = 0;

#if 0
typedef int (*my_ksys_write_t)(unsigned int fd, const char *buf, size_t count);
/* typedef void (*my_schedule_t)(void); */
typedef int (*my_tcp_sendmsg_t)(void *soc, void *msghdr, size_t count);

char msg_struct[96];
char kiocb_struct[48];

void *sym_sk = NULL;
struct iovec iov;
#endif

void do_write(int conn, char* data, int data_len){
  //Send the message back to client
  write(conn, data, data_len);

#if 0
  my_tcp_sendmsg_t my_tcp_sendmsg = (my_tcp_sendmsg_t) 0xffffffff81ab4e20;
  my_ksys_write_t my_ksys_write = (my_ksys_write_t) 0xffffffff8133e990;
  /* write(client_sock , client_message , strlen(client_message)); */
  iov.iov_base = (void *)data;
  iov.iov_len  = data_len;
  char msg_struct_c[96];

  if(use_shortcut){
    memcpy(msg_struct_c, msg_struct, 96);
    /* memcpy(kiocb_struct_c, kiocb_struct, 48); */
    /* iov_c = iov; */
    my_tcp_sendmsg(sym_sk, msg_struct_c, data_len);
  }else{
    my_ksys_write(conn , data , data_len);
  }
  /* my_schedule(); */
#endif
}


int main(int argc , char *argv[])
{
  /* my_schedule_t my_schedule = (my_schedule_t) 0xffffffff81bd12a0; */
	int socket_desc , client_sock , c , read_size;
	struct sockaddr_in server , client;
	char client_message[MSG_SZ];

  printf("server pid: %ld\n", (long)getpid());

	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 8888 );

	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	puts("Connection accepted");
  int my_ctr = 0;
	//Receive a message from client
#ifdef USE_SEND_RECV
	while( (read_size = recv(client_sock , client_message , MSG_SZ , 0)) > 0 )
#endif
#ifdef USE_READ_WRITE
  while( (read_size = read(client_sock , client_message , MSG_SZ)) > 0 )
#endif
	{
    my_ctr++;
    if((my_ctr %(1000) ) == 0){
      write(1, ".", 1);
    }
#ifdef USE_SEND_RECV
    send(client_sock , client_message , read_size, 0);
#endif
#ifdef USE_READ_WRITE
    /* write(client_sock , client_message , read_size); */
    do_write(client_sock , client_message , strlen(client_message));
#endif
	}

	if(read_size == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
	}
	else if(read_size == -1)
	{
		perror("recv failed");
	}

	return 0;
}