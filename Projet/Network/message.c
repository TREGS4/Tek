#include "message.h"
#include <unistd.h>
/*
*   Create a structure MESSAGE from the arguments.
*   If the allocation failed return null.
*   Return a pointer to the structure created.
*
*   The MESSAGE must be freed with DestroyMessage()
*/
MESSAGE *CreateMessage(int type, unsigned long long sizeData, char *data)
{
    MESSAGE *message = malloc(sizeof(MESSAGE));
    if (message == NULL)
        return NULL;

    message->data = malloc(sizeof(char) * sizeData);
    if (message->data == NULL)
    {
        free(message);
        return NULL;
    }

    message->type = type;
    message->sizeData = sizeData;
    memcpy(message->data, data, message->sizeData);

    return message;
}

/*
*   Transforma a binary message into a structure MESSAGE
*   if an error occurs return null, otherwise a pointer
*   to the structure MESSAGE is returned.
*
*   The MESSAGE must be freed with DestroyMessage()
*/
MESSAGE *BinToMessage(char *buff)
{
    int type = 0;
    unsigned long long sizeData = 0;
    char *data;

    memcpy(&type, buff, SIZE_TYPE_MSG);
    memcpy(&sizeData, buff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);

    data = malloc(sizeof(char) * sizeData);
    if(data == NULL)
        return NULL;
    memcpy(data, buff + HEADER_SIZE, sizeData);

    return CreateMessage(type, sizeData, data);
}

/*
*   Create a allocated buffer containing the all message (header and data) in bytes
*    WATCH OUT !! the message pass in argument is not freed.
*
*   The buffer can freed with free().
*/
char *MessageToBin(MESSAGE *message)
{
    char *res = malloc(sizeof(char) * (HEADER_SIZE + message->sizeData));
    char type = (char)message->type;
    memcpy(res, &type, SIZE_TYPE_MSG);
    memcpy(res + SIZE_TYPE_MSG, &message->sizeData, SIZE_DATA_LEN_HEADER);
    memcpy(res + HEADER_SIZE, message->data, message->sizeData);

    return res;
}

/*
*   Free properly the message pas in argument.
*/
void DestroyMessage(MESSAGE *message)
{
    free(message->data);
    free(message);
}

/*
*   Print the header and data of message in it's binary representation
*/
void printMessage(MESSAGE *message, struct sockaddr_in *IP)
{
    size_t taille = (3 + 10 + 21 + message->sizeData + sizeof(struct sockaddr_in)) * 10;
    char display[taille];
    char *mess = "Type: %d\nSize: %llu\nFrom: %s\nData: ";

    memset(display, 0, taille);

    char buffIP[16];
    memset(buffIP, 0, 16);
    if (IP != NULL)
        inet_ntop(AF_INET, &IP->sin_addr, buffIP, 16);

    sprintf(display, mess, message->type, message->sizeData, buffIP);
    size_t offset = strlen(display);

    for (size_t i = 0; i < message->sizeData * 3; i += 3)
    {
        sprintf(display + offset + i, "%02x ", message->data[i / 3]);
    }
    display[strlen(display)] = '\n';
    display[strlen(display)] = '\n';

    printf(display);
}