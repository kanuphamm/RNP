#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#define MAX_BUFFER_SIZE 1024
#define SRV_ADDRESS "127.0.0.1"
#define SRV_PORT "7777"

int main(int argc, char** argv) {
  /*if (argc < 3) {
    printf("Usage: %s <server_ipaddress> <server_port>\n", argv[0]);
    return 1;
  }*/

  const char* server_ipaddress = argv[1];
  int server_port = atoi(argv[2]);

  int s_tcp;
  struct addrinfo hints, *server_info, *p;
  int status;
  printf("Addrinfo added.\n");
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  /*if ((status = getaddrinfo(server_ipaddress, argv[2], &hints, &server_info)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }*/

    if ((status = getaddrinfo(SRV_ADDRESS, SRV_PORT, &hints, &server_info)) != 0) {
        fprintf(stderr, "Fehler bei getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

  // Iterate over the results and connect to the first working address
  for (p = server_info; p != NULL; p = p->ai_next) {
    if ((s_tcp = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("socket");
      continue;
    }

    if (connect(s_tcp, p->ai_addr, p->ai_addrlen) == -1) {
      perror("connect");
      close(s_tcp);
      continue;
    }
    printf("Connected successfully\n");
    break; // Connected successfully
  }

  if (p == NULL) {
    fprintf(stderr, "Failed to connect to the server\n");
    return 1;
  }

  freeaddrinfo(server_info);

  printf("Connected to the server.\n");
  printf("Enter commands ('Quit' to exit):\n");

  char command[MAX_BUFFER_SIZE];

  while (1) {
    fgets(command, MAX_BUFFER_SIZE, stdin);

    // Remove trailing newline character
    command[strcspn(command, "\n")] = '\0';

    if (strcmp(command, "Quit") == 0) {
      break;
    }

    ssize_t n = send(s_tcp, command, strlen(command), 0);
    if (n > 0) {
      printf("Message '%s' sent (%zi Bytes).\n", command, n);
    } else {
      perror("send");
      break;
    }

    printf("\nEnter commands ('Quit' to exit):\n");
  }

  close(s_tcp);
  return 0;
}
