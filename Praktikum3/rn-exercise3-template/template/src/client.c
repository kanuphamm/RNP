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
void receiveCommand(int sockfd);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

char* getAbsolutePath() {
    const char* relativePath = "../../src";
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
    return filePath;
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char command[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    FILE* file;
    char buffer[100];
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

    while (1) {
    fgets(command, MAXDATASIZE, stdin);

    // Remove newline character
    command[strcspn(command, "\n")] = '\0';
    ssize_t n = send(sockfd, command, strlen(command), 0);

// -----------------Command: Put
    if (strncmp(command, "Put", 3) == 0) {
        // Returns a pointer to the first occurrence of str2 in str1, or a null pointer if str2 is not part of str1.
        char *substring = strstr(command, "Put ");
        if (substring != NULL) {
            //get path
            char* absolutePath = getAbsolutePath();

            //substring points to beginning of "Put "
            char *filename = substring + 4; // Pointer to place after "Put "
            char* filePath = get_file_path(absolutePath, filename);


            file = fopen(filePath, "r");
            if (file == NULL) {
  //              perror("Fehler beim Öffnen der Datei ");
                printf("Fehler");
 //               exit(1);
            }
            memset(buffer, 0, sizeof buffer);
            while (fgets(buffer, sizeof(buffer), file) != NULL) {
                ssize_t n = send(sockfd, buffer, strlen(buffer), 0);
            }
            receiveCommand(sockfd);
            free(absolutePath);
            fclose(file);
        }
    }else if (strcmp(command, "Quit") == 0) {
        break;
    }


    if (n > 0) {
      printf("Message '%s' sent (%zi Bytes).\n", command, n);
    } else {
      perror("send");
      break;
    }

    printf("\nEnter commands ('Quit' to exit):\n");
  }

    close(sockfd);

    return 0;
}

void receiveCommand(int sockfd)
{
    ssize_t bytesRead;
    char buffer[MAX_BUFFER_SIZE];
    // Empfange und drucke die Serverantwort
    if((bytesRead = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        printf("%s", buffer);
        printf("Bytes gelesen: %ld\n", bytesRead);
    }

    if (bytesRead == -1) {
        perror("Fehler beim Empfangen der Daten");
    }
}