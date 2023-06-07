#include "cmdHandlerServer.h"

void handleListCommand(int *client_sockets, int num_clients, int sockfd, char* buffer, size_t bufferSize)
{
    int j;
    int messageLength = 0;
    memset(buffer, 0, bufferSize);
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
        messageLength += snprintf(buffer + messageLength, bufferSize - messageLength, "%s:%s\n", clientHost, clientPort);
    }

    // Append the number of clients to the message
    messageLength += snprintf(buffer + messageLength, bufferSize - messageLength, "%d Clients connected\n", num_clients);

    // Send the message to the desired socket
    send(sockfd, buffer, messageLength, 0);
    my_sendEOF(sockfd);
}



void handleFileCommand(const char* directory, int clientSocket, char* buffer, size_t bufferSize) {
    DIR* dir;
    struct dirent* entry;
    struct stat attrib;
    // Verzeichnis öffnen
    dir = opendir(directory);
    memset(buffer, 0, bufferSize);
    int cntDatein = 0;
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            // Den vollen Pfad zum Eintrag erstellen
            char pfad[bufferSize];
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
                memset(buffer, 0, bufferSize);
                cntDatein++;
            }
        }
        //<N> Dateien "%d daten",cntdaten
        sprintf(buffer, "%d Dateien\n", cntDatein);
        send(clientSocket, buffer, strlen(buffer), 0);
        memset(buffer, 0, bufferSize);
        // Verzeichnis schließen
        closedir(dir);
    } else {
        printf("Fehler beim Öffnen des Verzeichnisses\n");
    }
    fflush(stdout);
    my_sendEOF(clientSocket);//kann sein das hier wieder ein fehler auftritt
    return;
}



int handlePutCommand(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, size_t bufferSize, int sockfd, const  char* filename, const char* verzeichnis)
{   
    FILE *file;
    size_t length = strlen(verzeichnis) + strlen(filename) + 1; 
    char* pathAndFileName = (char*)malloc(length * sizeof(char));
    if (pathAndFileName == NULL) {
        printf("Memory allocation failed.\n");
    }
    strcpy(pathAndFileName, verzeichnis);
    strcat(pathAndFileName, filename);
    printf("<%s>\n",pathAndFileName);
    fflush(stdout);
    file = fopen(pathAndFileName, "w");
    if (file == NULL) {
        printf("Failed to open the file.\n");
    }
    fflush(stdout);
    my_recv(buf, sockfd, file);
    fclose(file);
    free(pathAndFileName);
    fflush(stdout);

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

    snprintf(message, sizeof(message), "OK %s\n%s\n%s\n\4", serverHost, serverIP, timeString);
    send(sockfd, message, sizeof(message), 0);
    //my_sendEOF(i); // in snprinf jetzt zu finden
    fflush(stdout);
    return 1;
}