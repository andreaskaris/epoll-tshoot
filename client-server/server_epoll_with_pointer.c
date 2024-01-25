/*
 * Adapted from https://www.educative.io/answers/how-to-implement-tcp-sockets-in-c
 *              https://suchprogramming.com/epoll-in-3-easy-steps/
 *              https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#define EPOLL_MAX_EVENTS 100
#define EPOLL_TIMEOUT    30000

static int run = 1;

void interruptHandler(int i) {
    run = 0;
}

int bind_socket(const char* ip, int port) {
    // Create socket, and set SO_REUSEPORT in to reuse bind port in case of
    // process crash and TIME-WAIT connections.
    struct sockaddr_in server_addr;
    int socket_fd;
    int optval = 1;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    
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

int handle_message(int fd) {
    ssize_t bytes_received;
    char server_message[2000], client_message[2000];
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));

    bytes_received = read(fd, client_message, sizeof(client_message)); 
    if (bytes_received < 0) {
        printf("Couldn't receive\n");
        return 1;
    }
    if (bytes_received == 0) {
        printf("Client closed connection\n");
        return 1;
    }
    // Act as a parrot.
    // sleep(5);
    strcpy(server_message, "RET: ");
    strcpy(server_message+strlen("RET: "), client_message);
    if (write(fd, server_message, strlen(server_message)) < 0){
        printf("Can't send\n");
        return 1;
    }
    return 0;
}

int epoll_ctl_add(int epoll_fd, int fd, uint32_t events) {
    struct epoll_event event;
    event.events = events;
    int * fd_ptr = malloc(sizeof(int));
    *fd_ptr = fd;
    // fprintf("fd_ptr is: %d\n", *fd_ptr);
    event.data.ptr = fd_ptr;
    // printf("event.data.ptr is: %d,*event.data.ptr is: %d\n", event.data.ptr,*((int*)event.data.ptr));
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event)) {
            fprintf(stderr, "Failed to add socket file descriptor to epoll\n");
            close(epoll_fd);
            return 1;
    }
    return 0;
}

void epoll_ctl_close(int epoll_fd, int fd) {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
}

int main(int argc, char **argv)
{
    if(argc != 3) {
        fprintf(stderr, "Please provide the listening address and port.\n");
        return 1;
    }
    char * address = argv[1];
    int port = atoi(argv[2]);
    // TODO: Add some validation for arguments.

    int socket_fd, client_sock, client_size;
    struct sockaddr_in client_addr;
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int err_code, event_count;

    signal(SIGINT, interruptHandler);

    // Create the epoll.
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
		fprintf(stderr, "Failed to create epoll file descriptor\n");
		return 1;
	}

    // Start listening on address <address> and port <port> and add socket to epoll.
    socket_fd = bind_socket(address, port);
    if(socket_fd == -1) {
            printf("Could not bind socket, %s\n", strerror(errno));
    }
    if(epoll_ctl_add(epoll_fd, socket_fd, EPOLLIN)) {
		    return 1;
    }


    while(run) {
            printf("Polling ...\n");
            event_count = epoll_wait(epoll_fd, events, EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
            if (event_count == -1) {
                fprintf(stderr, "epoll_wait failed with err: %s\n", strerror(errno));
            } else {
                printf("%d ready events\n", event_count);
            }
	      	for (int i = 0; i < event_count; i++) {
	            int events_data_fd = *(int*)events[i].data.ptr;
		    // printf("events[i].data.ptr is: %d, FD is %d\n", events[i].data.ptr, events_data_fd);
                    if(events_data_fd == socket_fd) {
                             client_size = sizeof(client_addr);
                             client_sock = accept(events_data_fd, (struct sockaddr*)&client_addr, &client_size);
                             if (client_sock < 0){
                                     fprintf(stderr, "Cannot accept failed client connection attempt\n");
                                     continue;
                             }
                             printf("Client connected at IP: %s and port: %i, fd: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_sock);
                             if(epoll_ctl_add(epoll_fd, client_sock, EPOLLIN | EPOLLRDHUP | EPOLLHUP)) {
                                 return 1;
                             }
                    } else if(events[i].events & EPOLLIN) {
                            // Handle this potentially in a thread.
                            // int pid = fork();
                            // if (pid == 0) {
                            handle_message(events_data_fd);
                            //  return 0;
                            // }
                    } else {
                            printf("unexpected events, %d\n", events[i].events) ;
                            printf("unexpected events, %d\n", events_data_fd);
                    }

                    if(events[i].events & (EPOLLRDHUP | EPOLLHUP)) {
                        printf("Closing connection with fd: %d.\n", events_data_fd);
                        epoll_ctl_close(epoll_fd, events_data_fd);
                    }
            }
    }

    // Close the listening socket and the epoll.
    close(socket_fd);
    if (close(epoll_fd)) {
		fprintf(stderr, "Failed to close epoll file descriptor\n");
		return 1;
	}
    
    return 0;
}
