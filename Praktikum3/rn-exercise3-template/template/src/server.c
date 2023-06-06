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


#define MAX_CLIENTS 25 // maximum number of clients
#define PORT "7777"   // port we're listening on
#define BUFFER_SIZE 1024

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

int handlePutCommand(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, int i, char* filename )
{   

    FILE *file;
    const char* verzeichnis = "../../src/storageServer/";
    size_t length = strlen(verzeichnis) + strlen(filename) + 1; //TODO frage +1
    char* pathAndFileName = (char*)malloc(length * sizeof(char));
    if (pathAndFileName == NULL) {
        printf("Memory allocation failed.\n");
        return -1;
    }
    strcpy(pathAndFileName, verzeichnis);
    strcat(pathAndFileName, filename);
    printf("<%s>",pathAndFileName);
    fflush(stdout);
    file = fopen(pathAndFileName, "w");
    if (file == NULL) {
    printf("Failed to open the file.\n");
        return -1;
    }

    int nbytes;
    int run = 1;
    char endOfFile = 4;
    while ( (run == 1) && (nbytes = recv(i, buf, BUFFER_SIZE -1, 0)) > 0  ) {
        
        char* eofPointer = strchr(buf, endOfFile);
        if (eofPointer != NULL) {
            printf("\nENDE File Erkannt!\n");
            run = 0;
            *eofPointer = '\0';
        }
        fprintf(file, "%s", buf);
        printf("\n<Recived: %s>\n",buf);
        fflush(stdout);
        fflush(file);
        memset(buf, 0, BUFFER_SIZE);
    }
    fclose(file);

    //Print Meta Data
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

    return 1;
}

void handleFileCommand(const char* directory, int clientSocket) {
    DIR* dir;
    struct dirent* entry;
    struct stat attrib;

    // Verzeichnis öffnen
    dir = opendir(directory);

    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            // Den vollen Pfad zum Eintrag erstellen
            char pfad[1024];
            sprintf(pfad, "%s/%s", directory, entry->d_name);

            // Dateiattribute abrufen
            stat(pfad, &attrib);

            // Nur reguläre Dateien berücksichtigen
            if (S_ISREG(attrib.st_mode)) {
                // Dateiname und Attribute in eine Zeichenkette formatieren
                char buffer[1024];
                sprintf(buffer, "%s\tLast Modified: %s\tSize: %lld bytes\n",
                        entry->d_name,
                        ctime(&attrib.st_mtime),
                        (long long)attrib.st_size);

                // Daten an den Clienten senden
                send(clientSocket, buffer, strlen(buffer), 0);
            }
        }

        // Verzeichnis schließen
        closedir(dir);
    } else {
        printf("Fehler beim Öffnen des Verzeichnisses\n");
        return;
    }
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
                    memset(buf, 0, BUFFER_SIZE);
                    if ((nbytes = recv(i, buf, BUFFER_SIZE -1, 0)) > 0) {//-1 ensures that there is space left for a null terminator at the end of the received data.
                     
                        // got error or connection closed by client
                        printf("Message received: %s\n", buf);

                        char delimiter[] = " \0\n";
                        char* token;
                        const int  numTokens = 2;
                        char** tokens = malloc(sizeof(char*) * numTokens);  // Speicher für maximal 2 Tokens reservieren
                        int cntTokens = 0;
                        
                        token = strtok(buf, delimiter);
                        while ( (token != NULL) && ( cntTokens < numTokens) ) {
                            size_t token_len = strlen(token);
                            printf("<strlen: token %ld>\n",token_len);
                            fflush(stdout);
                            tokens[cntTokens] = malloc(token_len + 1);  // Speicher für das Token reservieren
                            strcpy(tokens[cntTokens], token);
                            tokens[cntTokens][strcspn(tokens[cntTokens], "\n")] = '\0';
                            printf("GetToken:<%s>\n", tokens[cntTokens]);
                            fflush(stdout);
                            token = strtok(NULL, delimiter);
                            cntTokens++;
                        }

                        // -----------------Command: List                        
                        if (strcmp(tokens[0], "List") == 0) {
                            handleListCommand(client_sockets, num_clients);
                        // -----------------Command: Files                            
                        } else if (strcmp(tokens[0], "Files") == 0) {
                            const char* verzeichnis = "../../src/storageServer/";
                            handleFileCommand(verzeichnis, i); // i ist der clientSocket
                        } else if (strcmp(tokens[0], "Get") == 0) {
                            printf("Get\n");
                        // -----------------Command: Put                            
                        } else if (strcmp(tokens[0], "Put") == 0) {
                            handlePutCommand(remoteaddr, addrlen, buf, i, tokens[1]);
                        }

                        // Speicher für Tokens freigeben
                        for (int i = 0; i < cntTokens; i++) {
                            free(tokens[i]);
                        }
                        free(tokens);

                    } else {
                        if (nbytes == 0) {
                            // connection closed
                            printf("nbytes %d\n", nbytes);
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