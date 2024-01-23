/*
 * Adapted from https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int bind_socket(const char* ip, int port) {
    // Create socket:
    struct sockaddr_in server_addr;
    int socket_fd;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(socket_fd < 0){
        printf("Error while creating socket\n");
        return -1;
    }

    // Set port and IP:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);
    
    // Bind to the set port and IP:
    if(bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        printf("Couldn't bind to the port\n");
        return -1;
    }
    
    // Listen for clients:
    if(listen(socket_fd, 1) < 0){
        printf("Error while listening\n");
        return -1;
    }
    printf("Listening for incoming connections at IP: %s and port: %i\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));

    return socket_fd;
}

int handle_connection(int fd, struct sockaddr_in client_addr) {
    ssize_t bytes_received;
    char server_message[2000], client_message[2000];
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    for(;;) {
        bytes_received = recv(fd, client_message, sizeof(client_message), 0); 
        if (bytes_received < 0) {
            printf("Couldn't receive\n");
            return -1;
        }
        if (bytes_received == 0) {
            printf("Client closed connection at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            return 0;
        }
        // Act as a parrot.
        strcpy(server_message, client_message);
        if (send(fd, server_message, strlen(server_message), 0) < 0){
            printf("Can't send\n");
            return -1;
        }
    }
    
    // Closing the socket:
    close(fd);
    return 0;
}

int main(void)
{
    int socket_fd, client_sock, client_size;
    struct sockaddr_in client_addr;
    
    socket_fd = bind_socket("127.0.0.1", 1234);
    
    for(;;) {
        // Accept an incoming connection:
        client_size = sizeof(client_addr);
        client_sock = accept(socket_fd, (struct sockaddr*)&client_addr, &client_size);

        if (client_sock < 0){
            printf("Can't accept\n");
            continue;
        }
        printf("Client connected at IP: %s and port: %i\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        if (fork() == 0) {
            int err_code;
            err_code = handle_connection(client_sock, client_addr);
            return err_code;
        }
    }
    close(socket_fd);
    
    return 0;
}
