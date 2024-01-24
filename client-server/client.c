/*                                                                              
* Adapted from https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
*/  

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

int connect_socket(const char* ip, int port) {
    int socket_fd;
    struct sockaddr_in server_addr;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(socket_fd < 0){
        printf("Unable to create socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    // Send connection request to server:
    if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        printf("Unable to connect\n");
        return -1;
    }
    printf("Connected with server successfully\n");
    
    return socket_fd;
}

int send_message(int fd, const char* message) {
    char server_message[2000], client_message[2000];
    time_t timer;
    char tbuf[26];
    struct tm* tm_info;

    // Send the message to server:
    if(send(fd, message, strlen(message), 0) < 0){
        printf("Unable to send message\n");
        return -1;
    }
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(tbuf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] Sent:     %s\n", tbuf, message);
    
    // Receive the server's response:
    if(recv(fd, server_message, sizeof(server_message), 0) < 0){
        printf("Error while receiving server's msg\n");
        return -1;
    }
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(tbuf, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] Received: %s\n", tbuf, server_message);

    return 0;
}

int main(int argc, char **argv)
{
    if(argc != 3) {
        fprintf(stderr, "Please provide the address and port to connect to.\n");
        return 1;
    }
    char * address = argv[1];
    int port = atoi(argv[2]);
    // TODO: Add some validation for arguments.
    printf("Connecting to %s:%d\n", address, port);

    int socket_fd;
    char client_message[2000];
    memset(client_message,'\0',sizeof(client_message));
    
    socket_fd = connect_socket(address, port);
    if(socket_fd == -1) {
        return 1;
    }

    for(int i = 0;; i++) {
        sprintf(client_message, "%d", i);
        send_message(socket_fd, client_message);
        sleep(1);
    }
    close(socket_fd);

    return 0;
}
