#include "myHelperFunctions.h"

int my_sendEOF(int sockfd)
{
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


void my_recv(char* buf, size_t bufferSize, int sockfd, FILE *stream, int mode)
{
    if(mode == SAVE_MODE)
    {
        int nbytes;
        int run = 1;
        char endOfFile = 4;
        //TODO hier wen buff nicht leer erst buff lehren
        char* eofPointer = strchr(buf, endOfFile);
        if (eofPointer != NULL) {
            run = 0;
            *eofPointer = '\0';
        }
        fprintf(stream, "%s", buf);
        while ( (run == 1) && (nbytes = recv(sockfd, buf, bufferSize -1, 0)) > 0  ) {
            eofPointer = strchr(buf, endOfFile);
            if (eofPointer != NULL) {
                run = 0;
                *eofPointer = '\0';
            }
            fprintf(stream, "%s", buf);
            fflush(stream);
            memset(buf, 0, bufferSize);
        }
    }
    else if(mode == OVERWRITE_MODE)
    {
        int nbytes;
        int run = 1;
        char endOfFile = 4;
        memset(buf, 0, bufferSize);
        while ( (run == 1) && (nbytes = recv(sockfd, buf, bufferSize -1, 0)) > 0  ) 
        {
            char* eofPointer = strchr(buf, endOfFile);
            if (eofPointer != NULL) {
                run = 0;
                *eofPointer = '\0';
            }
            fprintf(stream, "%s", buf);
            fflush(stream);
            memset(buf, 0, bufferSize);
        }
    }
}

void trenneString(char* string, const char* trennzeichen, char** ersterTeil, char** zweiterTeil) 
{
    char* teil = strtok(string, trennzeichen);
    if (teil != NULL) {
        *ersterTeil = strdup(teil);
        
        teil = strtok(NULL, "");
        if (teil != NULL) {
            *zweiterTeil = strdup(teil);
        } else {
            *zweiterTeil = NULL;
        }
    } else {
        *ersterTeil = NULL;
        *zweiterTeil = NULL;
    }
}



char* getAbsolutePath( char * filepath) 
{ 
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



char* get_file_path(const char* absolutePath, const char* filename) 
{
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