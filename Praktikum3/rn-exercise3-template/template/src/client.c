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

void send_command(int sockfd, const char *command) {
    if (send(sockfd, command, strlen(command), 0) == -1)
        error("Error sending command to server");
}

void receive_response(int sockfd) {
    char buffer[BUFFER_SIZE];
    ssize_t n;

    while ((n = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[n] = '\0';
        printf("%s", buffer);

        if (buffer[n - 1] == EOT)
            break;
    }

    if (n == -1)
        error("Error receiving response from server");
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_address> <port>\n", argv[0]);
        exit(1);
    }

    const char *server_address = argv[1];
    int port = atoi(argv[2]);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        error("Error creating socket");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_address, &server_addr.sin_addr) <= 0)
        error("Invalid server address");

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        error("Error connecting to the server");

    char command[BUFFER_SIZE];

    while (1) {
        printf("Enter a command (List/Files/Get/Put/Quit): ");
        fgets(command, BUFFER_SIZE, stdin);

        // Remove trailing newline character
        command[strcspn(command, "\n")] = '\0';

        send_command(sockfd, command);
        receive_response(sockfd);

        if (strcmp(command, "Quit") == 0)
            break;
    }

    close(sockfd);

    return 0;
}
