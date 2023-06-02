#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdlib.h>

// TODO: Remove this block.
#define SRV_ADDRESS "127.0.0.1"
#define SRV_PORT 7777
#define MAX_BUFFER_SIZE 1024

int main(int argc, char** argv) {
  //const char *server_ipaddress = argv[1];  // TODO: Remove cast and parse arguments.
  //int server_port = atoi(argv[2]);  // TODO: Remove cast and parse arguments.

  FILE *file;

  int s_tcp; 
  struct sockaddr_in sa;
  unsigned int sa_len = sizeof(struct sockaddr_in);
  ssize_t bytesSent = 0;
  char* msg = "Hello all";

  sa.sin_family = AF_INET; //AF_INET = Adressfamilie, die für die Kommunikation über IP verwendet wird. Es handelt sich um die Standardadressfamilie für IPv4-Adressen.
  sa.sin_port = htons(SRV_PORT); //(host to network short)converts our byte order to network standard

  //converts ip address in string to binary "1.2.3.4" -> [1,2,3,4]
  if (inet_pton(sa.sin_family, SRV_ADDRESS, &sa.sin_addr.s_addr) <= 0) {
    perror("Address Conversion");
    return 1;
  }

  if ((s_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Erro rwhile creating TCP Socket");
    return 1;
  }

// Mit dem Server verbinden
  if (connect(s_tcp, (struct sockaddr*)&sa, sa_len) < 0) {
    perror("Connect");
    return 1;
  }
  printf("Connected to the server.\n");

  





  char buffer[MAX_BUFFER_SIZE];
  char buffer_stream[MAX_BUFFER_SIZE];
  while (1) {
    printf("\nEnter commands ('Quit' to exit):\n");
    fgets(buffer, MAX_BUFFER_SIZE, stdin);


    char delimiter[] = " ";

    char* token;
    char** tokens = malloc(sizeof(char*) * 10);  // Speicher für maximal 10 Tokens reservieren
    int numTokens = 0;

    token = strtok(buffer, delimiter);
    while (token != NULL) {
        tokens[numTokens] = malloc(strlen(token) /*+ 1*/);  // Speicher für das Token reservieren
        strcpy(tokens[numTokens], token);
        tokens[numTokens][strcspn(tokens[numTokens], "\n")] = '\0';
        printf("<%s>", tokens[numTokens]);

        token = strtok(NULL, delimiter);
        numTokens++;
    }

    if (strcmp(tokens[0], "List") == 0) {
      bytesSent = send(s_tcp, tokens[0], strlen(tokens[0]), 0);
      if (bytesSent < 0) {
          perror("Fehler beim Senden");
          exit(1);
      }
    }
    else if(strcmp(tokens[0], "Files") == 0){
      bytesSent = send(s_tcp, tokens[0], strlen(tokens[0]), 0);
      if (bytesSent < 0) {
          perror("Fehler beim Senden");
          exit(1);
      }
    }
    else if(strcmp(tokens[0], "Get") == 0){

    }
    else if(strcmp(tokens[0], "Put") == 0){
      printf("inside Put");
      // Datei öffnen
      file = fopen(tokens[1], "r");
      if (file == NULL) {
          perror("Fehler beim Öffnen der Datei ");
          exit(1);
      }

      // Calculate the length of the concatenated string
      char space[] = " ";
      size_t length = strlen(tokens[0]) + strlen(space) +strlen(tokens[1]) + 1;
    
      // Allocate memory for the concatenated string
      char* concatenated = (char*)malloc(length * sizeof(char));
      if (concatenated == NULL) {
        printf("Memory allocation failed.\n");
        return 1;
      }

      strcpy(concatenated, tokens[0]);
      strcat(concatenated, space);
      strcat(concatenated, tokens[1]);

      //Send Command Put <dateiname>
      bytesSent = send(s_tcp, concatenated, strlen(concatenated), 0);
      if (bytesSent < 0) {
          perror("Fehler beim Senden");
          exit(1);
      }

      // Datei zeilenweise lesen und an den Server senden
      while (fgets(buffer_stream, MAX_BUFFER_SIZE, file) != NULL) {
        bytesSent = send(s_tcp, buffer_stream, strlen(buffer_stream), 0);
        if (bytesSent < 0) {
            perror("Fehler beim Senden der Daten");
            exit(1);
        }
      }
    }
    else if(strcmp(tokens[0], "Quit") == 0){
      break;
    }
    

    // Speicher für Tokens freigeben
    for (int i = 0; i < numTokens; i++) {
        free(tokens[i]);
    }
    free(tokens);
    // Remove trailing newline character
    buffer[strcspn(buffer, "\n")] = '\0';

  }//end While
  
  close(s_tcp);
}