#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#define BUFFER_SIZE 1024

int main (int argc, char *argv[]) {

    int port = 5555;
    int server_fd, client_fd, err;
    struct sockaddr_in server, client;
    char buf[BUFFER_SIZE];

    // Zero buffer
    memset( buf, '\0', BUFFER_SIZE);

    // Server config
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = __builtin_bswap16 (port);
    server.sin_addr.s_addr = (uint32_t) 0x00000000;

    int opt_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof opt_val);

    err = bind(server_fd, (struct sockaddr *) &server, sizeof(server));
    err = listen(server_fd, 128);

    while (1) {
        socklen_t client_len = sizeof(client);
        client_fd = accept(server_fd, (struct sockaddr *) &client, &client_len);

        // 
        while (1) {
            int read = recvfrom(client_fd, buf, BUFFER_SIZE, 0, NULL, NULL);
            if(buf[0] == '\0') continue;
            err = send(client_fd, buf, read, 0);
                memset( buf, '\0', BUFFER_SIZE);
        }

    }
  return 0;
}
