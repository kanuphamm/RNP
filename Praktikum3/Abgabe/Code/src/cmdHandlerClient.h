#ifndef CMDHANDLERCLIENT_H
#define CMDHANDLERCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include "myHelperFunctions.h"

extern int handlePutCommand(char*filename,char* command, int sockfd, char* buffer_stream, size_t bufferSize);
extern void handleGetCommand(char* buffer_stream, size_t bufferSize, char* command, char*filename, const char* verzeichnis ,int sockfd);
#endif // CMDHANDLERCLIENT_H