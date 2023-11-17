#include <LINF/sym_all.h>
#include <stdio.h>
#include "kmod.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
    sym_elevate();
    __kmod_kprint("Hello from kernel world!\n");
    sym_lower();

    const char *server_ip = "127.0.0.1";
    int server_port = 8080;
    const char *message = "Hello from C client!";

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_address.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Connection failed");
        exit(1);
    }

    uint64_t user_stack;

    sym_elevate();
    //send(sock, message, strlen(message), 0);
    SYM_PRESERVE_USER_STACK(user_stack);
    SYM_SWITCH_TO_KERN_STACK();
    shortcut_tcp_sendmsg(sock, (void*)message, strlen(message));
    SYM_RESTORE_USER_STACK(user_stack);
    
    sym_lower();

    close(sock);
    return 0;
}
