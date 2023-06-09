#include "cmdHandlerServer.h"

void handleGetCommand(int sockfd,char* buffer, size_t bufferSize, char* filename, const char* verzeichnis){
    FILE* file;
    ssize_t bytesSent = 0;
                  
    char* absolutePath = getAbsolutePath("../../src/storageServer");
    char* filePath = get_file_path(absolutePath, filename);

    getCommandAnswer(verzeichnis, filename, sockfd, buffer, bufferSize);
    
    file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei ");
        free(absolutePath);
        free(filePath);
    }else{
        memset(buffer, 0, bufferSize);
        while (fgets(buffer, bufferSize, file) != NULL) {
            bytesSent = send(sockfd, buffer, strlen(buffer), 0);      
//            printf("Send: <%s>",buffer);
            fflush(stdout);
            if (bytesSent < 0) {
                perror("Fehler beim Senden der Daten");
            }
            memset(buffer, 0, bufferSize);
        }
        my_sendEOF(sockfd);
        free(absolutePath);
        free(filePath);
        fclose(file);
    }
}

void handleListCommand(int *client_sockets, int num_clients, int sockfd, char* buffer, size_t bufferSize)
{
    int j;
    int messageLength = 0;
    memset(buffer, 0, bufferSize);
    
    for (j = 0; j < num_clients; j++)
    {
        struct sockaddr_storage clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        // Netzwerkverbindungsinformationen abrufen
        getpeername(client_sockets[j], (struct sockaddr *)&clientAddr, &addrLen);

        char clientHost[NI_MAXHOST];
        char clientPort[NI_MAXSERV];

        // IP-Adresse und Hostname abrufen
        int result = getnameinfo((struct sockaddr *)&clientAddr, addrLen, clientHost, NI_MAXHOST, clientPort, NI_MAXSERV, 0/*NI_NUMERICHOST | NI_NUMERICSERV*/);

        if (result == 0) {
            // Hostname erfolgreich abgerufen
            //printf("Client %d: %s:%s\n", j+1, clientHost, clientPort);
        } else {
            // Fehler beim Abrufen des Hostnamens
            printf("Fehler beim Abrufen des Hostnamens für Client %d: %s\n", j+1, gai_strerror(result));
        }

        // Client-Informationen zur Nachricht hinzufügen
        messageLength += snprintf(buffer + messageLength, bufferSize - messageLength, "%s:%s\n", clientHost, clientPort);
    }

    // Die Anzahl der verbundenen Clients zur Nachricht hinzufügen
    messageLength += snprintf(buffer + messageLength, bufferSize - messageLength, "%d Clients connected\n\4", num_clients);

    // Die Nachricht an den gewünschten Socket senden
    send(sockfd, buffer, messageLength, 0);
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
                sprintf(buffer, "%s Datei-Attribute:\tLast Modified, %s\tSize, %lld bytes\n",
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

void getCommandAnswer(const char* directory, const char* filename, int clientSocket, char* buffer, size_t bufferSize) {
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
            sprintf(pfad, "%s%s", directory, entry->d_name);
            // Dateiattribute abrufen
            stat(pfad, &attrib);

            // Nur reguläre Dateien berücksichtigen und nach dem gewünschten Dateinamen suchen
            if (S_ISREG(attrib.st_mode) && strcmp(entry->d_name, filename) == 0) {
                // Dateiattribute in eine Zeichenkette formatieren
                sprintf(buffer, "Datei-Attribute: Last Modified - %s, Size - %lld bytes\n\4",
                        ctime(&attrib.st_mtime),
                        (long long)attrib.st_size);

                // Daten an den Clienten senden
                send(clientSocket, buffer, strlen(buffer), 0);
                memset(buffer, 0, bufferSize);
                cntDatein++;
                break; // Die Schleife abbrechen, nachdem die Datei gefunden wurde
            }
        }
        if (cntDatein == 0) {
            // Datei nicht gefunden
            sprintf(buffer, "Datei '%s' nicht gefunden\n\4", filename);
            send(clientSocket, buffer, strlen(buffer), 0);
            memset(buffer, 0, bufferSize);
        }
        // Verzeichnis schließen
        closedir(dir);
    } else {
        printf("Fehler beim Öffnen des Verzeichnisses\n");
    }
    fflush(stdout);
    return;
}


int handlePutCommand(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, size_t bufferSize, int sockfd, const  char* filename, const char* verzeichnis, char* ip)
{   
    FILE *file;
    size_t length = strlen(verzeichnis) + strlen(filename) + 1; 
    char* pathAndFileName = (char*)malloc(length * sizeof(char));
    if (pathAndFileName == NULL) {
        printf("Memory allocation failed.\n");
        fflush(stdout);
    }
    strcpy(pathAndFileName, verzeichnis);
    strcat(pathAndFileName, filename);

    file = fopen(pathAndFileName, "w");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        fflush(stdout);
    }
    
    my_recv(buf, bufferSize, sockfd, file, SAVE_MODE);
    fclose(file);
    free(pathAndFileName);

    //Print Meta Data
    time_t currentTime;
    struct tm *timeInfo;
    char timeString[20];
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeInfo);

    char serverHost[NI_MAXHOST];
    char serverPort[NI_MAXHOST];
    getnameinfo((struct sockaddr *)&remoteaddr, addrlen, serverHost, NI_MAXHOST, serverPort, NI_MAXHOST, 0);

    char message[512];

    struct sockaddr_in* sa = (struct sockaddr_in*)&remoteaddr;
    char serverIPP[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(sa->sin_addr), serverIPP, INET_ADDRSTRLEN);

    printf("Server-IP-Adresse: %s\n", serverIPP);

    snprintf(message, sizeof(message), "OK %s\n%s\n%s\n\4", serverHost, ip, timeString);
    send(sockfd, message, sizeof(message), 0);
    //my_sendEOF(i); // in snprinf jetzt zu finden
    fflush(stdout);
    return 1;
}

int handlePutCommandHost(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, size_t bufferSize, int sockfd, const  char* filename, const char* verzeichnis, char* ip)
{
    FILE *file;
    size_t length = strlen(verzeichnis) + strlen(filename) + 1;
    char* pathAndFileName = (char*)malloc(length * sizeof(char));
    if (pathAndFileName == NULL) {
        printf("Memory allocation failed.\n");
        fflush(stdout);
    }
    strcpy(pathAndFileName, verzeichnis);
    strcat(pathAndFileName, filename);

    file = fopen(pathAndFileName, "w");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        fflush(stdout);
    }

    my_recv(buf, bufferSize, sockfd, file, SAVE_MODE);
    fclose(file);
    free(pathAndFileName);

    //Print Meta Data
    time_t currentTime;
    struct tm *timeInfo;
    char timeString[20];
    time(&currentTime);
    timeInfo = localtime(&currentTime);
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeInfo);

    char serverHost[NI_MAXHOST];
    char serverPort[NI_MAXHOST];
    getnameinfo((struct sockaddr *)&remoteaddr, addrlen, serverHost, NI_MAXHOST, serverPort, NI_MAXHOST, NI_NUMERICHOST);

    char message[512];

    struct sockaddr_in* sa = (struct sockaddr_in*)&remoteaddr;
    char serverIPP[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(sa->sin_addr), serverIPP, INET_ADDRSTRLEN);

    printf("Server-IP-Adresse: %s\n", serverIPP);
    char hostname[256];
    if (gethostname(hostname, bufferSize) == 0) {
        printf("host name %s", hostname);
    } else {
        perror("gethostname");
        return -1; // Fehler
    }

    snprintf(message, sizeof(message), "OK %s\n%s\n%s\n\4", hostname, serverHost, timeString);
    //snprintf(message, sizeof(message), "OK %s\n%s\n%s\n\4", serverHost, ip, timeString);
    send(sockfd, message, sizeof(message), 0);
    //my_sendEOF(i); // in snprinf jetzt zu finden
    fflush(stdout);
    return 1;
}