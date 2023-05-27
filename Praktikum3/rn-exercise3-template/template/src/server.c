#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// TODO: Remove me.
#define SRV_PORT 7777

int main(int argc, char** argv) {
  (void)argc;  // TODO: Remove cast and parse arguments.
  (void)argv;  // TODO: Remove cast and parse arguments.
  int s_tcp, news;
  struct sockaddr_in sa, sa_client;
  unsigned int sa_len = sizeof(struct sockaddr_in);
  char info[256];

  sa.sin_family = AF_INET;
  sa.sin_port = htons(SRV_PORT);
  sa.sin_addr.s_addr = INADDR_ANY;

  if ((s_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("TCP Socket");
    return 1;
  }

  if (bind(s_tcp, (struct sockaddr*)&sa, sa_len) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(s_tcp, 5) < 0) {
    perror("listen");
    close(s_tcp);
    return 1;
  }
  // TODO: Check port in use and print it.
  printf("Waiting for TCP connections ... \n");

  while (1) {
    if ((news = accept(s_tcp, (struct sockaddr*)&sa_client, &sa_len)) < 0) {
      perror("accept");
      close(s_tcp);
      return 1;
    }
    if (recv(news, info, sizeof(info), 0)) {
      printf("Message received: %s \n", info);
    }
  }

  close(s_tcp);
}