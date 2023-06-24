#ifndef CMDHANDLERSERVER_H
#define CMDHANDLERSERVER_H

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
#include "myHelperFunctions.h"

extern void handleGetCommand(int sockfd,char* buffer, size_t bufferSize, char* filename, const char* verzeichnis);
extern void handleListCommand(int *client_sockets, int num_clients, int sockfd, char* buffer, size_t bufferSize);
extern void handleFileCommand(const char* directory, int clientSocket, char* buffer, size_t bufferSize);
extern int handlePutCommand(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, size_t bufferSize, int sockfd, const  char* filename, const char* verzeichnis, char* ip);
extern void getCommandAnswer(const char* directory, const char* filename, int clientSocket, char* buffer, size_t bufferSize);
extern int handlePutCommandHost(struct sockaddr_storage remoteaddr, socklen_t addrlen, char* buf, size_t bufferSize, int sockfd, const  char* filename, const char* verzeichnis, char* ip);

#endif // CMDHANDLERSERVER_H