#include "cmdHandlerClient.h"

int handlePutCommand(char*filename,char* command, int sockfd, char* buffer_stream, size_t bufferSize)
{
    FILE* file;
    ssize_t bytesSent = 0;

    // Datei öffnen                    
    char* absolutePath = getAbsolutePath("../../src/storageClient");
    //printf("\nabsolutePath: %s\n",absolutePath);
    //substring points to beginning of "Put "
    char* filePath = get_file_path(absolutePath, filename);
    //printf("filePath: %s\n",filePath);

    file = fopen(filePath, "r");
    if (file == NULL) {
        perror("Fehler beim Öffnen der Datei ");
        free(absolutePath);
        free(filePath);
        return -1;
    }else{
        // Calculate the length of the concatenated string
        char space[] = " ";
        char EOT[] = "\4";
        size_t length = strlen(command) + strlen(space) +strlen(filename)+ strlen(EOT)+ 1; //TODO frage +1

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
        strcat(concatenated, EOT);

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
        while (fgets(buffer_stream, bufferSize, file) != NULL) {
            bytesSent = send(sockfd, buffer_stream, strlen(buffer_stream), 0);
            printf("Send: %s",buffer_stream);
            if (bytesSent < 0) {
                perror("Fehler beim Senden der Daten");
            }
        }
        my_sendEOF(sockfd);
        free(absolutePath);
        free(filePath);
        fclose(file);
        
    }
    return 1;
}