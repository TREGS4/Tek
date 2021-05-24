#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <err.h>
#include <arpa/inet.h>

#include "informations.h"

/*
*   Structure MESSAGE containing the header (type and sizeData) and the
    data of the message.

    IMPROVEMENT
    add information about the transmitter
*/

typedef struct
{
    int type;
    unsigned long long sizeData;
    char *data;
} MESSAGE;

/*
*   Create a structure MESSAGE from the arguments.
*   If the allocation failed return null.
*   Return a pointer to the structure created.
*
*   The MESSAGE must be freed with DestroyMessage()
*/
MESSAGE *CreateMessage(int type, unsigned long long sizeData, char *data);

/*
*   Free properly the message pas in argument.
*/
void DestroyMessage(MESSAGE *message);

/*
*   Create a allocated buffer containing the all message (header and data) in bytes
*    WATCH OUT !! the message pass in argument is not freed.
*
*   The buffer can freed with free().
*/
char *MessageToBin(MESSAGE *message);

/*
*   Transforma a binary message into a structure MESSAGE
*   if an error occurs return null, otherwise a pointer
*   to the structure MESSAGE is returned.
*
*   The MESSAGE must be freed with DestroyMessage()
*/
MESSAGE *BinToMessage(char *buff);

/*
*   Print the header and data of message in it's binary representation
*/
void printMessage(MESSAGE *message, struct sockaddr_in *IP);

#endif