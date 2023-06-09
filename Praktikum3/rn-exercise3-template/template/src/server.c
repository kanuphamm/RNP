#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include "cmdHandlerServer.h"
#include "myHelperFunctions.h"


#define MAX_CLIENTS 25 // maximum number of clients
#define PORT "7777"   // port we're listening on
#define BUFFER_SIZE 1024

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    printf("connet detected!\n");
    if (sa->sa_family == AF_INET) {
        printf("IPv4 detected!\n");
        return &(((struct sockaddr_in*)sa)->sin_addr);
    } else if (sa->sa_family == AF_INET6) {
        printf("IPv6 detected!\n");
        return &(((struct sockaddr_in6*)sa)->sin6_addr);
    }

    return NULL;
}

int main(void)
{
    static const char* verzeichnis = "../../src/storageServer/"; //speicherverzeichnis für Files

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[BUFFER_SIZE];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, rv;

    struct addrinfo hints, *ai, *p;

    int client_sockets[MAX_CLIENTS]; // array to store client sockets
    int num_clients = 0; // number of connected clients

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    //binde netzwerk addresse an socket
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);


    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one
    printf("Waiting for TCP connections ... \n");
    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                        (struct sockaddr *)&remoteaddr,
                        &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        //add client to list
                        client_sockets[num_clients] = newfd;
                        num_clients++;

                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
                            inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*)&remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                            newfd);
                    }
                } else {
// -----------------handle data from a client
                    memset(buf, 0, BUFFER_SIZE);
                    if ((nbytes = recv(i, buf, BUFFER_SIZE -1, 0)) > 0) {//-1 ensures that there is space left for a null terminator at the end of the received data.
//                        printf("Message received: <%s>\n", buf);
                                                   
                            // got error or connection closed by client
                            

                            const int numTokens = 4;
                            char** tokens = malloc(sizeof(char*) * numTokens);
                            char* originalString = malloc(nbytes + 1); // Speicher für den ursprünglichen String reservieren
                            strcpy(originalString, buf); // Den ursprünglichen String kopieren
                            trenneString(originalString, "\4", &tokens[0], &tokens[1]);  
                            trenneString(tokens[0], " ", &tokens[2], &tokens[3]);                 
                                                                 // Beispiel:
//                          printf("Token 0: <%s>\n", tokens[0]);// Commando +\4+ Dateiname
//                          printf("Token 1: <%s>\n", tokens[1]);// optional File Inhalt eventuel auch \4 NULL wen nicht mit drinn
                            printf("Token 2 (Command): <%s>\n", tokens[2]);// Put
                            printf("Token 3 (Dateiname): <%s>\n", tokens[3]);// dateiname
                            fflush(stdout);

                            if(tokens[1] != NULL){
                                strncpy(buf, tokens[1], BUFFER_SIZE - 1);
                                buf[BUFFER_SIZE - 1] = '\0';  // Stelle sicher, dass der Buffer ordnungsgemäß abgeschlossen ist
//                              printf("Kopierter String (Token[1]->buf): %s\n", buf);
                            }else{
                                memset(buf, 0, BUFFER_SIZE);
                            }
// -----------------Command: List    
                            if(tokens[2] != NULL)
                            {                   
                                if (strcmp(tokens[2], "List") == 0) {
                                    handleListCommand(client_sockets, num_clients, i,buf, BUFFER_SIZE);
// -----------------Command: Files                            
                                } else if (strcmp(tokens[2], "Files") == 0) {
                                    handleFileCommand(verzeichnis, i, buf, BUFFER_SIZE); // i ist der clientSocket
// -----------------Command: Get                                       
                                } else if (strcmp(tokens[2], "Get") == 0) {
                                handleGetCommand(i, buf, BUFFER_SIZE, tokens[3],verzeichnis);
// -----------------Command: Put                            
                                } else if (strcmp(tokens[2], "Put") == 0) {
                                    //handlePutCommand(remoteaddr, addrlen, buf, BUFFER_SIZE, i, tokens[3], verzeichnis, remoteIP);
                                    handlePutCommandHost(remoteaddr, addrlen, buf, BUFFER_SIZE, i, tokens[3], verzeichnis, remoteIP);
                                }
                            }
                            // Speicher für Tokens freigeben
                            for (int i = 0; i < numTokens; i++) {//TODO
                                free(tokens[i]);
                            }
                            free(tokens);
                    } else {
                        if (nbytes == 0) {
                            // connection closed
                            printf("\nnbytes %d\n", nbytes);
                            printf("Client disconnected on socket %d.\n", i);
                        } else {
                            perror("recv");
                        }
                        num_clients--;
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } 
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}