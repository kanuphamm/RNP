#ifndef CMDHANDLERCLIENT_H
#define CMDHANDLERCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#define MAX_BUFFER_SIZE 1024

char* getAbsolutePath(char* filepath);
char* get_file_path(const char* absolutePath, const char* filename);
int my_sendEOF(int sockfd);
void my_recv(char* buf, int sockfd, FILE* stream);
int handlePutCommand(char* filename, char* command, int sockfd, char* buffer_stream);

#endif // CMDHANDLERCLIENT_H