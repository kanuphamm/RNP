#include "cmdHandlerServer.h"
#define BUFFER_SIZE 1024

/**
 * Handle the "list" command by retrieving information about connected clients and sending it to the desired socket.
 *
 * @param client_sockets An array of client sockets.
 * @param num_clients The number of connected clients.
 * @param sockfd The socket descriptor to send the message.
 * @param message The buffer to store the generated message.
 */
void handleListCommand(int *client_sockets, int num_clients, int sockfd,char* message)
{
    int j;
    int messageLength = 0;
    memset(message, 0, BUFFER_SIZE);
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

        // Append client information to the message
        messageLength += snprintf(message + messageLength, BUFFER_SIZE - messageLength, "%s:%s\n", clientHost, clientPort);
    }

    // Append the number of clients to the message
    messageLength += snprintf(message + messageLength, BUFFER_SIZE - messageLength, "%d Clients connected\n", num_clients);

    // Send the message to the desired socket
    send(sockfd, message, messageLength, 0);
    my_sendEOF(sockfd);
}


/**
 * Handle the "put" command by receiving a file from a client and saving it to the server.
 *
 * @param remoteaddr The address information of the client.
 * @param addrlen The length of the address structure.
 * @param buf The buffer to receive the file data.
 * @param i The socket descriptor of the client.
 * @param filename The name of the file.
 * @return Returns 1 on success.
 */
int handlePutCommand(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, int i, char* filename )
{   

    FILE *file;
    const char* verzeichnis = "../../src/storageServer/";
    size_t length = strlen(verzeichnis) + strlen(filename) + 1; //TODO frage +1
    char* pathAndFileName = (char*)malloc(length * sizeof(char));
    if (pathAndFileName == NULL) {
        printf("Memory allocation failed.\n");
    }
    strcpy(pathAndFileName, verzeichnis);
    strcat(pathAndFileName, filename);
    printf("<%s>",pathAndFileName);
    fflush(stdout);
    file = fopen(pathAndFileName, "w");
    if (file == NULL) {
    printf("Failed to open the file.\n");
    }
    my_recv(buf, i, file);
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

    char message[512];

    snprintf(message, sizeof(message), "OK %s\n%s\n%s\n", serverHost, serverIP, timeString);
    send(i, message, sizeof(message), 0);
    my_sendEOF(i);
    return 1;
}


/**
 * Handle the "file" command by listing files in the specified directory and sending the information to the client.
 *
 * @param directory The directory to scan for files.
 * @param clientSocket The socket descriptor of the client.
 * @param buffer The buffer to store the file information.
 */
void handleFileCommand(const char* directory, int clientSocket, char* buffer) {
    DIR* dir;
    struct dirent* entry;
    struct stat attrib;
    fflush(stdout);
    // Verzeichnis öffnen
    dir = opendir(directory);
    memset(buffer, 0, BUFFER_SIZE);
    int cntDatein = 0;
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            fflush(stdout);
            // Den vollen Pfad zum Eintrag erstellen
            char pfad[BUFFER_SIZE];
            sprintf(pfad, "%s/%s", directory, entry->d_name);

            // Dateiattribute abrufen
            stat(pfad, &attrib);

            // Nur reguläre Dateien berücksichtigen
            if (S_ISREG(attrib.st_mode)) {
                // Dateiname und Attribute in eine Zeichenkette formatieren
                sprintf(buffer, "%s\tLast Modified: %s\tSize: %lld bytes\n",
                        entry->d_name,
                        ctime(&attrib.st_mtime),
                        (long long)attrib.st_size);

                // Daten an den Clienten senden
                send(clientSocket, buffer, strlen(buffer), 0);
                memset(buffer, 0, BUFFER_SIZE);
                cntDatein++;
            }
        }
        //<N> Dateien "%d daten",cntdaten
        sprintf(buffer, "%d Dateien", cntDatein);
        send(clientSocket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        // Verzeichnis schließen
        closedir(dir);
    } else {
        printf("Fehler beim Öffnen des Verzeichnisses\n");
    }
    fflush(stdout);
    my_sendEOF(clientSocket);
    return;
}


/**
 * Empfängt Daten vom Socket und schreibt sie in den angegebenen Stream,
 * bis das End-of-File-Zeichen (EOF) im empfangenen Puffer gefunden wird.
 *
 * @param buf Der Puffer zum Speichern der empfangenen Daten.
 * @param sockfd Der Socket-Dateideskriptor zum Empfangen von Daten.
 * @param stream Der Stream, in den die empfangenen Daten geschrieben werden sollen.
 */
void my_recv(char* buf, int sockfd, FILE *stream){
    int nbytes;
    int run = 1;
    char endOfFile = 4;
    memset(buf, 0, BUFFER_SIZE);
    while ( (run == 1) && (nbytes = recv(sockfd, buf, BUFFER_SIZE -1, 0)) > 0  ) {
        char* eofPointer = strchr(buf, endOfFile);
        if (eofPointer != NULL) {
            run = 0;
            *eofPointer = '\0';
        }
        fprintf(stream, "%s", buf);
        fflush(stream);
        memset(buf, 0, BUFFER_SIZE);
    }
}


/**
 * Sendet das End-of-File-Zeichen (EOF) über den angegebenen Socket.
 *
 * @param sockfd Der Socket-Dateideskriptor zum Senden des EOF-Zeichens.
 * @return Die Anzahl der erfolgreich gesendeten Bytes oder -1 bei einem Fehler.
 */
int my_sendEOF(int sockfd){
    ssize_t bytesSent = 0;
    char endOfFile = '\4';
    char* ptrEndOfFile = &endOfFile;
    bytesSent = send(sockfd, ptrEndOfFile, sizeof(char), 0);
    if (bytesSent < 0) {
        perror("Fehler beim Senden endOfLine");
        return -1;
    }
    return bytesSent;
}


/**
 * Überprüft, ob das End-of-File-Zeichen (EOF) im angegebenen Puffer enthalten ist.
 *
 * @param buf Der Puffer, der auf das EOF-Zeichen überprüft werden soll.
 * @return 1, wenn das EOF-Zeichen gefunden wurde, ansonsten -1.
 */
int my_checkBufEOF(char* buf){
    char endOfFile = 4;
    char* eofPointer = strchr(buf, endOfFile);
    if (eofPointer != NULL) {
        //eofPointer = " ";
        return 1;
    }

    return -1;
}