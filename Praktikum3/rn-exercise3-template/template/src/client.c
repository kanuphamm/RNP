#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <limits.h>
#include <arpa/inet.h>
#include "cmdHandlerClient.h"

#define PORT "7777" // the port client will be connecting to 
#define SRV_ADDRESS "127.0.0.1"
#define MAXDATASIZE 1024 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}




int main(int argc, char *argv[])
{
    printf("argc: %d",argc);
    printf("argv[0]: %s",argv[0]);
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    ssize_t bytesSent = 0;
    
    /*if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }*/

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(SRV_ADDRESS, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    printf("Connected to the server.\n");
    printf("Enter commands ('Quit' to exit):\n");

    char buffer[MAX_BUFFER_SIZE];
    char buffer_stream[MAX_BUFFER_SIZE];

//WHILE--------------------------------------------
    int ClientIsRunning = 1;
    while (ClientIsRunning) {
        printf("\nEnter commands ('Quit' to exit):\n");
        if(fgets(buffer, MAX_BUFFER_SIZE, stdin) == NULL ){
            perror("fehler fgets stdin");
        }

        char delimiter[] = " ";
        char* token;
        char** tokens = malloc(sizeof(char*) * 2);  // Speicher für maximal 2 Tokens reservieren
        int numTokens = 0;

        token = strtok(buffer, delimiter);
        while (token != NULL) {
            tokens[numTokens] = malloc(strlen(token) + 1);  // Speicher für das Token reservieren
            strcpy(tokens[numTokens], token);
            tokens[numTokens][strcspn(tokens[numTokens], "\n")] = '\0';
            printf("GetToken<%s>", tokens[numTokens]);

            token = strtok(NULL, delimiter);
            numTokens++;
        }

//Command Manager------------------------------------
        if (strcmp(tokens[0], "List") == 0) {
            bytesSent = send(sockfd, tokens[0], strlen(tokens[0]), 0);
            my_sendEOF(sockfd);
            my_recv(buffer_stream, sockfd, stdout);
        }
        else if(strcmp(tokens[0], "Files") == 0){
            bytesSent = send(sockfd, tokens[0], strlen(tokens[0]), 0);
            my_sendEOF(sockfd);

            my_recv(buffer_stream, sockfd, stdout);
        }
        else if(strcmp(tokens[0], "Get") == 0){

        }
        else if(strcmp(tokens[0], "Put") == 0 && numTokens == 2) {
            handlePutCommand( tokens[1], tokens[0], sockfd, buffer_stream);
            my_recv(buffer_stream, sockfd, stdout);
        }
        else if(strcmp(tokens[0], "Quit") == 0){
            ClientIsRunning = 0;
        }

        // Speicher für Tokens freigeben
        for (int i = 0; i < numTokens; i++) {
            free(tokens[i]);
        }
        free(tokens);
        // Remove trailing newline character
        buffer[strcspn(buffer, "\n")] = '\0';

    }//end While

    printf("\nEnter commands ('Quit' to exit):\n");


    close(sockfd);

    return 0;
}