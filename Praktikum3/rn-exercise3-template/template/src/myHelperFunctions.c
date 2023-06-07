#include "myHelperFunctions.h"

void trenneString(char* string, const char* trennzeichen, char** ersterTeil, char** zweiterTeil) 
{
    char* teil = strtok(string, trennzeichen);
    if (teil != NULL) {
        *ersterTeil = strdup(teil);
        
        teil = strtok(NULL, "");
        if (teil != NULL) {
            *zweiterTeil = strdup(teil);
        } else {
            *zweiterTeil = NULL;
        }
    } else {
        *ersterTeil = NULL;
        *zweiterTeil = NULL;
    }
}