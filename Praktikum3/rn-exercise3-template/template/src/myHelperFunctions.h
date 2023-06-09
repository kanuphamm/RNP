#ifndef MYHELPERFUNCTIONS_H
#define MYHELPERFUNCTIONS_H


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

#define SAVE_MODE 1
#define OVERWRITE_MODE 0
#define PRINT_LINE_FILE printf("Zeile: %d, Datei: %s\n", __LINE__, __FILE__)

extern int my_sendEOF(int sockfd);
extern void my_recv(char* buf,size_t bufferSize, int sockfd, FILE *stream, int mode);

extern void trenneString(char* string, const char* trennzeichen, char** ersterTeil, char** zweiterTeil); 
extern char* getAbsolutePath( char * filepath);
extern char* get_file_path(const char* absolutePath, const char* filename); 

#endif // MYHELPERFUNCTIONS_H