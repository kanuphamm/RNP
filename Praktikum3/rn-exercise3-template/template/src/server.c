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


#define MAX_CLIENTS 25 // maximum number of clients
#define PORT "7777"   // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void handleListCommand(int *client_sockets, int num_clients)
{
    int j;
    for (j = 0; j < num_clients; j++)
    {
        struct sockaddr_storage clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        //get info of network connection
        getpeername(client_sockets[j], (struct sockaddr *)&clientAddr, &addrLen);

        char clientHost[NI_MAXHOST];
        char clientPort[NI_MAXSERV];

        //get ip
        getnameinfo((struct sockaddr *)&clientAddr, addrLen, clientHost, NI_MAXHOST, clientPort, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        printf("%s:%s\n", clientHost, clientPort);
    }
    printf("%d Clients connected\n", num_clients);
}

void handlePutCommand(struct sockaddr_storage remoteaddr, socklen_t addrlen)
{
    time_t currentTime;
    struct tm *timeInfo;
    char timeString[20];
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeInfo);

    char serverHost[NI_MAXHOST];
    char serverIP[NI_MAXHOST];
    getnameinfo((struct sockaddr *)&remoteaddr, addrlen, serverHost, NI_MAXHOST, serverIP, NI_MAXHOST, NI_NUMERICHOST);

    printf("OK %s\n", serverHost);
    printf("%s\n", serverIP);
    printf("%s\n", timeString);
}

int main(void)
{
    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[1024];    // buffer for client data
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    int client_sockets[MAX_CLIENTS]; // array to store client sockets
    int num_clients = 0; // number of connected clients

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
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
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

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
                    memset(buf, 0, sizeof(buf));
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) > 0) {
                        // got error or connection closed by client
                        printf("Message received: %s\n", buf);
                        if (strcmp(buf, "List") == 0) {
// -----------------Command: List
                            handleListCommand(client_sockets, num_clients);
                        } else if (strcmp(buf, "Files") == 0) {
                            printf("Files\n");
                        } else if (strcmp(buf, "Get") == 0) {
                            printf("Get\n");
                        } else if (strcmp(buf, "Put") == 0) {
// -----------------Command: Put
                            handlePutCommand(remoteaddr, addrlen);
                        }
                    } else {
                        if (nbytes == 0) {
                            // connection closed
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