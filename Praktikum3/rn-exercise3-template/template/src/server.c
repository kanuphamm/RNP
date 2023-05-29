#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <netdb.h>

// TODO: Remove me.
#define SRV_PORT 7777

int main(int argc, char** argv) {
  (void)argc;  // TODO: Remove cast and parse arguments.
  (void)argv;  // TODO: Remove cast and parse arguments.
  int s_tcp, news;
  struct sockaddr_in sa, sa_client;
  unsigned int sa_len = sizeof(struct sockaddr_in);
  char info[1024];

  sa.sin_family = AF_INET;
  sa.sin_port = htons(SRV_PORT);
  sa.sin_addr.s_addr = INADDR_ANY;

  if ((s_tcp = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Error in TCP Socket");
    return 1;
  }
  //initialize fd set
  fd_set current_sockets, ready_sockets;
  FD_ZERO(&current_sockets); // clears out the fd_set, so that it doesn't contain any file descriptors. 
  FD_SET(s_tcp, &current_sockets); //Füge den TCP-Socket s_tcp dem aktuellen Set der zu überwachenden Sockets hinzu

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
          // Clear the buffer before receiving a new message
          memset(info, 0, sizeof(info));
          if (recv(news, info, sizeof(info)-1, 0) > 0) {
            //TODO handle commands
            printf("Message received: %s\n", info);
            if(strcmp(info, "List") == 0) {
              printf("List\n");
            } else if (strcmp(info, "Files") == 0){
              printf("Files\n");
            } else if (strcmp(info, "Get") == 0){
              printf("Get\n");           
            } else if (strcmp(info, "Put") == 0){
              printf("Put\n");             
            }
        } else {
          // Error or connection closed by the client
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
}