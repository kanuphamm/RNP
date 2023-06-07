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

extern void trenneString(char* string, const char* trennzeichen, char** ersterTeil, char** zweiterTeil); 

#endif // MYHELPERFUNCTIONS_H