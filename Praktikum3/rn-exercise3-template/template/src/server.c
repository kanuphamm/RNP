#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

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
    perror("Error in TCP Socket");
    return 1;
  }
  //initialize fd set
  fd_set current_sockets, ready_sockets;
  FD_ZERO(&current_sockets);
  FD_SET(s_tcp, &current_sockets);

  //Socket wird an Adresse und Port des Servers gebunden
  if (bind(s_tcp, (struct sockaddr*)&sa, sa_len) < 0) {
    perror("bind error");
    return 1;
  }

  //fängt an zu hören
  if (listen(s_tcp, 5) < 0) {
    perror("listen error");
    close(s_tcp);
    return 1;
  }
  // TODO: Check port in use and print it.
  printf("Waiting for TCP connections ... \n");

  while (1) {
    ready_sockets = current_sockets;

    if(select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0){
      perror("Select error");
      close(s_tcp);
      return 1;
    }

    for(int i = 0; i < FD_SETSIZE; i++){
      if(FD_ISSET(i, &ready_sockets)){
        if(i == s_tcp){
          //this is a new connection
          if ((news = accept(s_tcp, (struct sockaddr*)&sa_client, &sa_len)) < 0) {
            perror("accept");
            close(s_tcp);
            return 1;
          }
          FD_SET(news, &current_sockets);
        }else{
          news = i;
        if (recv(news, info, sizeof(info), 0) > 0) {
          printf("Message received: %s\n", info);
          //TODO handle commands
        } else {
          // Error or connection closed by the client
          if (recv == 0) {
            printf("Client disconnected.\n");
          } else {
            printf("Client disconnected.\n");
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
}