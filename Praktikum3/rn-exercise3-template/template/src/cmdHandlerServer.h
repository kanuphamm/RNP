#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

//handle command
void handleListCommand(int *client_sockets, int num_clients, int sockfd, char* message);
int handlePutCommand(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, int i, char* filename);
void handleFileCommand(const char* directory, int clientSocket, char* buffer);

//Receive command
void my_recv(char* buf, int sockfd, FILE *stream);

//EOF
int my_sendEOF(int sockfd);
int my_checkBufEOF(char* buf);
