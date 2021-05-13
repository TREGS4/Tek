#include "server.h"
#include "client.h"

void Send(int fd, const void *buf, size_t count, int flag)
{
	ssize_t r = 0;
	size_t reste = count;

	while (r >= 0 && (r = send(fd, buf + count - reste, count, flag)) < (long int)reste)
		reste -= r;

	if (r < 0)
		err(3, "Error while rewriting");
}

/*Return the size of the date in byte + the size of the headar in byte*/
unsigned long long sizeMessage(char *message)
{
	unsigned long long size;
	memcpy(&size, message + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);
	return size + HEADER_SIZE;
}

int connectClient(struct sockaddr_in *IP)
{
	int skt = -1;

	if (IP == NULL)
		return -1;

	if ((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	if (connect(skt, (struct sockaddr *)IP, sizeof(struct sockaddr_in)) < 0)
	{
		printf("Connect client error:\n");
		perror(NULL);
		return -1;
	}

	return skt;
}

int SendMessageForOneClient(struct clientInfo *client, char *message, unsigned long long len)
{
	int skt = -1;
	struct sockaddr_in clienttemp;

    clienttemp.sin_addr = client->IPandPort.sin_addr;
    clienttemp.sin_family = client->IPandPort.sin_family;
    clienttemp.sin_port = htons(atoi(PORT));

	if ((skt = connectClient(&clienttemp)) < 0)
	{
		printf("Error while creating the socket\n");
		return -1;
	}

	Send(skt, message, len, 0);
	close(skt);
	return 0;
}

int SendMessage(struct clientInfo *clientList, char *message)
{
	for (clientList = clientList->sentinel->next; clientList != clientList->sentinel; clientList = clientList->next)
		SendMessageForOneClient(clientList, message, sizeMessage(message));

	return 1;
}

int addClient(struct sockaddr_in IP, struct clientInfo *clientList)
{
	if(itsme(&IP, &clientList->server->IPandPort) == 0 && isInList(&IP, clientList) == NULL)
	{
		struct clientInfo *client = initClient(clientList);
		client->IPandPort = IP;
		client->IPLen = sizeof(struct sockaddr_in);
		printf("Client add\n");
	}

	return 1;
}
