#include "client.h"
#include "server.h"

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
	unsigned long long size = 0;
	memcpy(&size, message + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);
	return size + HEADER_SIZE;
}

int connectClient(struct address address)
{
	int skt = -1;

	if ((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	printf("Addr: %s\nPort: %s\n", address.hostname, address.port);
	struct sockaddr_in IP = GetIPfromHostname(address);
	printIP(&IP);

	if (connect(skt, (struct sockaddr *)&IP, sizeof(struct sockaddr_in)) < 0)
	{
		printf("Connect client error:\n");
		perror(NULL);
		return -1;
	}

	return skt;
}

int SendMessageForOneClient(struct clientInfo *client, MESSAGE message)
{
	int skt = -1;

	if ((skt = connectClient(client->address)) < 0)
		return -1;

	char *binMessage = MessageToBin(message);
	Send(skt, binMessage, HEADER_SIZE + message.sizeData, 0);
	free(binMessage);
	close(skt);
	return 0;
}

int SendMessage(struct clientInfo *clientList, MESSAGE message)
{
	for (clientList = clientList->sentinel->next; clientList->isSentinel == FALSE; clientList = clientList->next)
	{
		if (SendMessageForOneClient(clientList, message) < 0)
		{
			struct clientInfo *temp = clientList;
			clientList = clientList->next;
			removeClient(temp);
		}
	}

	return 1;
}
