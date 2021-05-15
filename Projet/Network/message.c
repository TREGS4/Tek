#include "message.h"
#include "informations.h"

MESSAGE CreateMessage(int type, unsigned long long sizeData, char *data)
{
    MESSAGE message;

    message.type = type;
    message.sizeData = sizeData;

    message.data = malloc(sizeof(char) * message.sizeData);
    memcpy(message.data, data, message.sizeData);

    return message;
}

MESSAGE BinToMessage(char *buff)
{
    MESSAGE message;
    char type;
    memcpy(&type, buff, SIZE_TYPE_MSG);
    message.type = (int)type;
    memcpy(&message.sizeData, buff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);

    message.data = malloc(sizeof(char) * message.sizeData);
    memcpy(message.data, buff + HEADER_SIZE, message.sizeData);

    return message;
}

char *MessageToBin(MESSAGE message)
{
    char *res = malloc(sizeof(char) * (HEADER_SIZE + message.sizeData));
    char type = (char)message.type;
    memcpy(res, &type, SIZE_TYPE_MSG);
    memcpy(res + SIZE_TYPE_MSG, &message.sizeData, SIZE_DATA_LEN_HEADER);
    memcpy(res + HEADER_SIZE, message.data, message.sizeData);

    return res;
}

void DestroyMessage(MESSAGE message)
{
    free(message.data);
}

void printMessage(MESSAGE message)
{
	size_t taille = (3 + 10 + 21 + message.sizeData) * 10;
    char *display = malloc(sizeof(char) * taille);
    memset(display, 0, taille);

	sprintf(display, "Type: %d\nSize: %llu\nData: ", message.type, message.sizeData);
	size_t offset = strlen(display);

	for (size_t i = 0; i < message.sizeData * 3; i += 3)
	{
		sprintf(display + offset + i, "%02x ", message.data[i/3]);
	}
	display[strlen(display)] = '\n';
	display[strlen(display)] = '\n';

	printf(display);
}