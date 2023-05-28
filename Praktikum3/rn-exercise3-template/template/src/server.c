#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define EOT 4

void error(const char *message) {
    perror(message);
    exit(1);
}

void send_response(int sockfd, const char *response) {
    if (send(sockfd, response, strlen(response), 0) == -1)
        error("Error sending response to client");

    // Send EOT character to mark end of transmission
    if (send(sockfd, &EOT, 1, 0) == -1)
        error("Error sending EOT character to client");
}

void handle_list_command(int sockfd) {
    const char *response = "Client1:12345\nClient2:23456\n2 Clients connected\n";
    send_response(sockfd, response);
}

void handle_files_command(int sockfd) {
    const char *response = "file1.txt last modified: 2023-05-27, size: 1024\nfile2.txt last modified: 2023-05-26, size: 2048\n2 Files\n";
    send_response
