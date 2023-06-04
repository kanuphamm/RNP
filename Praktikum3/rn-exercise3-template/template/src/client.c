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

#define PORT "7777" // the port client will be connecting to 
#define SRV_ADDRESS "127.0.0.1"
#define MAXDATASIZE 1024 // max number of bytes we can get at once 
#define MAX_BUFFER_SIZE 1024

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* getAbsolutePath( char * filepath) { //"../../src/storageClient"
    const char* relativePath = filepath;
    char* path = (char*)malloc(PATH_MAX * sizeof(char));

// Get the current working directory
    if (getcwd(path, PATH_MAX) != NULL) {
        // Get the absolute path of the specified directory
        char* absolutePath = realpath(relativePath, NULL);
        if (absolutePath != NULL) {
            // Append a trailing slash if necessary
            size_t len = strlen(absolutePath);
            if (len > 0 && absolutePath[len - 1] != '/') {
                char* newPath = (char*)malloc((len + 2) * sizeof(char));
                strcpy(newPath, absolutePath);
                strcat(newPath, "/");
                free(absolutePath);
                absolutePath = newPath;
            }

            free(path);
            return absolutePath;
        } else {
            perror("Error getting the absolute path");
            free(path);
            return NULL;
        }
    } else {
        perror("Error getting the current working directory");
        free(path);
        return NULL;
    }
}

char* get_file_path(const char* absolutePath, const char* filename) {
    // Calculate the required buffer size for the concatenated string
    int bufferSize = snprintf(NULL, 0, "%s%s", absolutePath, filename);
    if (bufferSize < 0) {
        // Error handling if snprintf fails
        fprintf(stderr, "Error calculating buffer size.\n");
        return NULL;
    }
    // Allocate memory for the concatenated string
    char* filePath = malloc(bufferSize + 1);
    if (filePath == NULL) {
        // Error handling if memory allocation fails
        fprintf(stderr, "Error allocating memory.\n");
        return NULL;
    }

    // Concatenate the absolute path and filename
    snprintf(filePath, bufferSize + 1, "%s%s", absolutePath, filename);
    printf("filepath: %s\n", filePath);
    return filePath;
}

int handlePutCommand(char*filename,char* command, int sockfd, char* buffer_stream)
{
    FILE* file;
    ssize_t bytesSent = 0;

    // Datei öffnen                    
    char* absolutePath = getAbsolutePath("../../src/storageClient");
    printf("\nabsolutePath: %s\n",absolutePath);
    //substring points to beginning of "Put "
    char* filePath = get_file_path(absolutePath, filename);
    printf("filePath: %s\n",filePath);

    file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei ");
        free(absolutePath);
        free(filePath);
        return -1;
    }else{
        // Calculate the length of the concatenated string
        char space[] = " ";
        size_t length = strlen(command) + 2*strlen(space) +strlen(filename) + 1; //TODO frage +1

        // Allocate memory for the concatenated string
        char* concatenated = (char*)malloc(length * sizeof(char));
        if (concatenated == NULL) {
            printf("Memory allocation failed.\n");
                free(absolutePath);
                free(filePath);
                fclose(file);
            return -1;
        }

        strcpy(concatenated, command);
        strcat(concatenated, space);
        strcat(concatenated, filename);
        strcat(concatenated, space);

        //Send Command Put <dateiname>
        bytesSent = send(sockfd, concatenated, strlen(concatenated), 0);
        if (bytesSent < 0) {
            perror("Fehler beim Senden");
                free(absolutePath);
                free(filePath);
                fclose(file);
            return -1;
        }
        free(concatenated);

        // Datei zeilenweise lesen und an den Server senden
        while (fgets(buffer_stream, MAX_BUFFER_SIZE, file) != NULL) {
            bytesSent = send(sockfd, buffer_stream, strlen(buffer_stream), 0);
            printf("Send: %s",buffer_stream);
            if (bytesSent < 0) {
                perror("Fehler beim Senden der Daten");
                free(absolutePath);
                free(filePath);
                fclose(file);
                return -1;
            }
        }
        bytesSent = send(sockfd, "EOF", strlen("EOF"), 0);
        if (bytesSent < 0) {
            perror("Fehler beim Senden der Daten");
            free(absolutePath);
            free(filePath);
            fclose(file);
            return -1;
        }
        free(absolutePath);
        free(filePath);
        fclose(file);
        
    }
    return 1;
}

void handleFilesCommand(int sockfd)
{
    ssize_t bytesRead;
    char buffer[MAX_BUFFER_SIZE];

    // Empfange und drucke die Serverantwort
    while ((bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
    }

    if (bytesRead == -1) {
        perror("Fehler beim Empfangen der Daten");
    }
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
            if (bytesSent < 0) {
                perror("Fehler beim Senden");
                exit(1);
            }
        }
        else if(strcmp(tokens[0], "Files") == 0){
            bytesSent = send(sockfd, tokens[0], strlen(tokens[0]), 0);
            if (bytesSent < 0) {
                perror("Fehler beim Senden");
                exit(1);
            }
            handleFilesCommand(sockfd);
        }
        else if(strcmp(tokens[0], "Get") == 0){

        }
        else if(strcmp(tokens[0], "Put") == 0 && numTokens == 2) {
            handlePutCommand( tokens[1], tokens[0], sockfd, buffer_stream);
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