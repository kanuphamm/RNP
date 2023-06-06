#include "cmdHandlerClient.h"


/**
 * Gibt den absoluten Dateipfad für einen gegebenen relativen Dateipfad zurück.
 *
 * @param filepath Der relative Dateipfad.
 * @return Der absolute Dateipfad oder NULL bei einem Fehler.
 */
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


/**
 * Gibt den vollständigen Dateipfad durch Verknüpfen des absoluten Pfads und des Dateinamens zurück.
 *
 * @param absolutePath Der absolute Dateipfad.
 * @param filename Der Dateiname.
 * @return Der vollständige Dateipfad oder NULL bei einem Fehler.
 */
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


/**
 * Sendet das EOF-Zeichen an den Server.
 *
 * @param sockfd Der Socket-Deskriptor.
 * @return Die Anzahl der gesendeten Bytes oder -1 bei einem Fehler.
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
 * Empfängt Daten vom Server und schreibt sie in einen Stream (Datei).
 *
 * @param buf Der Puffer zum Speichern der empfangenen Daten.
 * @param sockfd Der Socket-Deskriptor.
 * @param stream Der Ziel-Stream (Datei).
 */
void my_recv(char* buf, int sockfd, FILE *stream){
    int nbytes;
    int run = 1;
    char endOfFile = 4;
    memset(buf, 0, MAX_BUFFER_SIZE);
    while ( (run == 1) && (nbytes = recv(sockfd, buf, MAX_BUFFER_SIZE -1, 0)) > 0  ) {
        char* eofPointer = strchr(buf, endOfFile);
        if (eofPointer != NULL) {
            run = 0;
            *eofPointer = '\0';
        }
        fprintf(stream, "%s", buf);
        fflush(stream);
        memset(buf, 0, MAX_BUFFER_SIZE);
    }
}


/**
 * Behandelt den "Put"-Befehl, um eine Datei an den Server zu senden.
 *
 * @param filename Der Dateiname.
 * @param command Der Befehl.
 * @param sockfd Der Socket-Deskriptor.
 * @param buffer_stream Der Puffer zum Speichern der Dateidaten.
 * @return 1 bei Erfolg, -1 bei einem Fehler.
 */
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
        size_t length = strlen(command) + strlen(space) +strlen(filename) + 1; //TODO frage +1

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

        my_sendEOF(sockfd);

        // Datei zeilenweise lesen und an den Server senden
        while (fgets(buffer_stream, MAX_BUFFER_SIZE, file) != NULL) {
            if(bytesSent = send(sockfd, buffer_stream, strlen(buffer_stream), 0) > 0){
            printf("Send: %s",buffer_stream);
            }
                else if(bytesSent < 0) {
                    perror("Fehler beim Senden der Daten");
                    free(absolutePath);
                    free(filePath);
                    fclose(file);
                    return -1;
                }
        }
        my_sendEOF(sockfd);
        free(absolutePath);
        free(filePath);
        fclose(file);
    }
    return 1;
}
