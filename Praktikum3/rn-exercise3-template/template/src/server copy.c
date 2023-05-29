#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>

#define SRV_PORT "7777"
#define MAX_BUFFER_SIZE 1024

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    int server_socket, new_socket;
    struct addrinfo hints, *server_info, *p;
    struct sockaddr_storage client_address;
    socklen_t address_length = sizeof(struct sockaddr_storage);
    char info[MAX_BUFFER_SIZE];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE;     // Use local IP address

    // Resolve the server address and port
    int result = getaddrinfo(NULL, SRV_PORT, &hints, &server_info);
    if (result != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(result));
        return 1;
    }

    // Create a socket for each address returned by getaddrinfo
    for (p = server_info; p != NULL; p = p->ai_next) {
        server_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (server_socket < 0) {
            perror("Error creating socket");
            continue;
        }

        // Bind the socket to the address and port
        if (bind(server_socket, p->ai_addr, p->ai_addrlen) < 0) {
            perror("Error binding socket");
            close(server_socket);
            continue;
        }

        break; // Socket creation and binding successful
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to create and bind socket\n");
        return 1;
    }

    freeaddrinfo(server_info); // No longer needed

    // Start listening for incoming connections
    if (listen(server_socket, 5) < 0) {
        perror("Error listening for connections");
        close(server_socket);
        return 1;
    }

    // Set up the fd_set for monitoring sockets
    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets); // Clear the fd_set
    FD_SET(server_socket, &current_sockets); // Add the server socket to the current set of monitored sockets

    printf("Waiting for TCP connections...\n");

    while (1) {
        ready_sockets = current_sockets;

        // Monitor the sockets for activity
        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
            perror("Select error");
            close(server_socket);
            return 1;
        }

        // Iterate over all sockets in the monitoring list
        for (int i = 0; i < FD_SETSIZE; i++) {
            if (FD_ISSET(i, &ready_sockets)) {
                if (i == server_socket) {
                    // This is a new connection
                    if ((new_socket = accept(server_socket, (struct sockaddr*)&client_address, &address_length)) < 0) {
                        perror("Accept error");
                        close(server_socket);
                        return 1;
                    }

                    // Add the new socket to the set of monitored sockets
                    FD_SET(new_socket, &current_sockets);
                } else {
                    // This is an already connected socket
                    int socket_fd = i;

                    // Clear the buffer before receiving a new message
                    memset(info, 0, sizeof(info));

                    // Receive data from the client
                    ssize_t received_bytes = recv(socket_fd, info, sizeof(info) - 1, 0);
                    if (received_bytes > 0) {
                        // TODO: Handle the received commands
                        printf("Message received: %s\n", info);
                        if (strcmp(info, "List") == 0) {
                            printf("List\n");
                        } else if (strcmp(info, "Files") == 0) {
                            printf("Files\n");
                        } else if (strcmp(info, "Get") == 0) {
                            printf("Get\n");
                        } else if (strcmp(info, "Put") == 0) {
                            printf("Put\n");
                        }
                    } else {
                        // Error or connection closed by the client
                        if (received_bytes == 0) {
                            printf("Client disconnected.\n");
                        } else {
                            perror("recv");
                        }
                        close(socket_fd);
                        FD_CLR(socket_fd, &current_sockets);
                    }
                }
            }
        }
    }

    close(server_socket);
    return 0;
}
