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
  const char *server_ipaddress = argv[1];  // TODO: Remove cast and parse arguments.
  int server_port = atoi(argv[2]);  // TODO: Remove cast and parse arguments.
  int s_tcp;
  struct sockaddr_in sa;
  unsigned int sa_len = sizeof(struct sockaddr_in);
  ssize_t n = 0;
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

  if (connect(s_tcp, (struct sockaddr*)&sa, sa_len) < 0) {
    perror("Connect");
    return 1;
  }

  printf("Connected to the server.\n");
  printf("Enter commands ('Quit' to exit):\n");
    
  char command[MAX_BUFFER_SIZE];

  while (1) {
    fgets(command, MAX_BUFFER_SIZE, stdin);
        
    // Remove trailing newline character
    command[strcspn(command, "\n")] = '\0';

    if ((n = send(s_tcp, command, strlen(command), 0)) > 0) {
      printf("Message %s sent (%zi Bytes).\n", command, n);
    }

    if (strcmp(command, "Quit") == 0) {
            break;
    }
    printf("\nEnter commands ('Quit' to exit):\n");
  }
  close(s_tcp);
}