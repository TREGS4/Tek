#include "message.h"
#include <unistd.h>
/*
*   Create a structure MESSAGE from the arguments.
*   If the allocation failed return null.
*   Return a pointer to the structure created.
*
*   The MESSAGE must be freed with DestroyMessage()
*/
MESSAGE *CreateMessage(int type, unsigned long long sizeData, BYTE *data)
{
    MESSAGE *message = malloc(sizeof(MESSAGE));
    if (message == NULL)
        return NULL;

    message->data = malloc(sizeof(BYTE) * sizeData);
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
MESSAGE *BinToMessage(BYTE *buff)
{
    int type = 0;
    unsigned long long sizeData = 0;

    memcpy(&type, buff, SIZE_TYPE_MSG);
    memcpy(&sizeData, buff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);

    return CreateMessage(type, sizeData, buff + HEADER_SIZE);
}

/*
*   Create a allocated buffer containing the all message (header and data) in bytes
*    WATCH OUT !! the message pass in argument is not freed.
*
*   The buffer can freed with free().
*/
BYTE *MessageToBin(MESSAGE *message)
{
    BYTE *res = malloc(sizeof(BYTE) * (HEADER_SIZE + message->sizeData));
    BYTE type = (BYTE)message->type;
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
    size_t taille = (5 + 10 + 21 + message->sizeData + sizeof(struct sockaddr_in)) * 10;
    char display[taille];
    char *mess = "Type: %d\nSize: %llu\nFrom: %s:%u\nData: ";

    memset(display, 0, taille);

    char buffIP[16];
    memcpy(buffIP, "NONE ", 5);
    uint16_t port = 0;
    if (IP != NULL){
        memset(buffIP, 0, 16);
        if (IP != NULL)
        {
            inet_ntop(AF_INET, &IP->sin_addr, buffIP, 16);
            port = ntohs(IP->sin_port);
        }
    }
        
    sprintf(display, mess, message->type, message->sizeData, buffIP, port);
    size_t offset = strlen(display);

    for (size_t i = 0; i < message->sizeData * 5; i += 5)
    {
        sprintf(display + offset + i, "%04x ", message->data[i / 5]);
    }
    display[strlen(display)] = '\n';
    display[strlen(display)] = '\n';

    printf("%s", display);
}