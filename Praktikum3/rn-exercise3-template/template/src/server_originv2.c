#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <netdb.h>

#define SRV_PORT "7777"

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("Usage: %s <hostname>\n", argv[0]);
    return 1;
  }

  int s_tcp, news;
  struct addrinfo hints, *res, *p;
  struct sockaddr_storage sa_client;
  socklen_t sa_len = sizeof(struct sockaddr_storage);
  char info[1024];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;  // Allow IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, SRV_PORT, &hints, &res) != 0) {
    perror("getaddrinfo");
    return 1;
  }

  for (p = res; p != NULL; p = p->ai_next) {
    s_tcp = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (s_tcp < 0) {
      perror("socket");
      continue;
    }

    if (bind(s_tcp, p->ai_addr, p->ai_addrlen) < 0) {
      close(s_tcp);
      perror("bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "Failed to bind socket\n");
    return 1;
  }

  freeaddrinfo(res);

  if (listen(s_tcp, 5) < 0) {
    perror("listen");
    close(s_tcp);
    return 1;
  }

  printf("Waiting for TCP connections ... \n");

  while (1) {
    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets);
    FD_SET(s_tcp, &current_sockets);
    ready_sockets = current_sockets;

    if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0) {
      perror("select");
      close(s_tcp);
      return 1;
    }

    for (int i = 0; i < FD_SETSIZE; i++) {
      if (FD_ISSET(i, &ready_sockets)) {
        if (i == s_tcp) {
          news = accept(s_tcp, (struct sockaddr*)&sa_client, &sa_len);
          if (news < 0) {
            perror("accept");
            close(s_tcp);
            return 1;
          }
          FD_SET(news, &current_sockets);
        } else {
          news = i;
          memset(info, 0, sizeof(info));
          if (recv(news, info, sizeof(info) - 1, 0) > 0) {
            printf("Message received: %s\n", info);
            if (strcmp(info, "List") == 0) {
              printf("List\n");
            } else if (strcmp(info, "Files") == 0) {
              printf("Files\n");
            } else if (strcmp(info, "Get") == 0) {
              printf("Get\n");
            } else if (strcmp(info, "Put") == 0) {
              printf("Put\n");
            }
          } else {
            if (recv == 0) {
              printf("Client disconnected.\n");
            } else {
              perror("recv");
            }
            close(news);
            FD_CLR(news, &current_sockets);
          }
        }
      }
    }
  }

  close(s_tcp);
  return 0;
}
