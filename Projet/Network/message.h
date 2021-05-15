#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct
{
    int type;
    unsigned long long sizeData;
    char *data;
} MESSAGE;

MESSAGE CreateMessage(int type, unsigned long long sizeData, char *data);
void DestroyMessage(MESSAGE message);
char *MessageToBin(MESSAGE message);
MESSAGE BinToMessage(char *buff);
void printMessage(MESSAGE message, struct sockaddr_in *IP);

#endif