#include "server.h"
#include "network_tools.h"

void *read_thread(void *arg)
{
	//printf("in read thread server\n");
	struct clientInfo *client = arg;
	int fdin = client->clientSocket;
	int fdout = client->fdoutExtern;
	int writingExtern = 0;
	int writingIntern = 0;
	unsigned long long size = 0;
	int type = -1;

	char buffLen[SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG + 1];
	char buffType[SIZE_TYPE_MSG + 1];
	char buff[BUFFER_SIZE_SOCKET];

	while (client->status == CONNECTED)
	{
		memset(buffLen, 0, SIZE_DATA_LEN_HEADER + 1);
		memset(buffType, 0, SIZE_TYPE_MSG + 1);
		memset(buff, 0, BUFFER_SIZE_SOCKET);

		int r = 1;

		size_t nbToRead = SIZE_DATA_LEN_HEADER + SIZE_TYPE_MSG;
		size_t nbchr = 0;

		/*Header part*/

		while (client->status != ERROR && client->status != ENDED && nbToRead > 0)
		{
			if ((r = read(fdin, &buffLen + nbchr, nbToRead)) < 0)
			{
				pthread_mutex_lock(&client->lockInfo);
				client->status = ERROR;
				printf("An error has occured when reading the header on the socket\n");
				pthread_mutex_unlock(&client->lockInfo);
			}
			else if (r == 0)
			{
				pthread_mutex_lock(&client->lockInfo);
				client->status = ENDED;
				pthread_mutex_unlock(&client->lockInfo);
			}

			nbToRead -= r;
			nbchr += r;
		}

		for (size_t i = 0; i < SIZE_TYPE_MSG; i++)
			buffType[i] = buffLen[i];

		type = buffLen[0];
		memcpy(&size, &buffLen[SIZE_TYPE_MSG], 8);

		if (size < BUFFER_SIZE_SOCKET)
			nbToRead = size;
		else
			nbToRead = BUFFER_SIZE_SOCKET;

		if (type == 1) //block the corresponding mutex for intern or extern message and setup the correct fdout
		{
			pthread_mutex_lock(client->lockReadGlobalIntern);
			writingIntern = 1;
			fdout = client->fdoutIntern;
		}
		else
		{
			pthread_mutex_lock(client->lockReadGlobalExtern);
			writingExtern = 1;
			fdout = client->fdoutExtern;
		}

		if (client->status == CONNECTED)
			write(fdout, buffLen, SIZE_TYPE_MSG + SIZE_DATA_LEN_HEADER);

		/*Message part*/

		while (client->status != ERROR && client->status != ENDED && size > 0)
		{
			if ((r = read(fdin, &buff, nbToRead)) < 0)
			{
				pthread_mutex_lock(&client->lockInfo);
				client->status = ERROR;
				printf("An error has occured when reading the data on the socket\n");
				pthread_mutex_unlock(&client->lockInfo);
			}
			else if (r == 0)
			{
				pthread_mutex_lock(&client->lockInfo);
				client->status = ENDED;
				pthread_mutex_unlock(&client->lockInfo);
			}

			size -= r;

			if (size > BUFFER_SIZE_SOCKET)
				nbToRead = size;
			else
				nbToRead = BUFFER_SIZE_SOCKET;

			write(fdout, buff, r);
		}

		if (writingExtern == 1)
			pthread_mutex_unlock(client->lockReadGlobalExtern);
		if (writingIntern == 1)
			pthread_mutex_unlock(client->lockReadGlobalIntern);
	}

	return NULL;
}

void *client_thread(void *arg)
{
	struct clientInfo *client = arg;

	if (pthread_create(&client->readThread, NULL, read_thread, arg) == 0)
	{
		pthread_mutex_lock(&client->lockInfo);
		client->status = CONNECTED;
		pthread_mutex_unlock(&client->lockInfo);
		pthread_join(client->readThread, NULL);
	}
	else
	{
		printf("Error while create the read thread for :\n");
		printIP(&client->IPandPort);
	}

	pthread_mutex_lock(&client->lockRead);
	pthread_mutex_lock(&client->lockWrite);

	close(client->clientSocket);

	pthread_mutex_unlock(&client->lockWrite);
	pthread_mutex_unlock(&client->lockRead);

	pthread_mutex_lock(&client->lockInfo);
	if(client->status == CONNECTED)
		client->status = NOTCONNECTED;
	pthread_mutex_unlock(&client->lockInfo);

	return NULL;
}

/*
	This section create the server and listen for connection.
	When connection is receive, a fork is made for the client.
	Each fork contain two thread for transmit and receive data simultanously.
*/
void *server(void *arg)
{
	struct serverInfo *serInfo = arg;
	struct addrinfo hints;
	struct addrinfo *res;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	int connect = 0;
	int skt;
	int finish = 0;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, PORT, &hints, &res);

	while (res != NULL && connect == 0)
	{
		int value = 1;
		skt = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if (skt >= 0)
		{
			setsockopt(skt, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
			setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

			if (bind(skt, res->ai_addr, res->ai_addrlen) == 0)
				connect = 1;
			else
				close(skt);
		}
		else
			res = res->ai_next;
	}

	freeaddrinfo(res);

	if (skt < 0)
		err(EXIT_FAILURE, "Error while creating the socket in server.c");
	if (listen(skt, 5) == -1)
		err(EXIT_FAILURE, "Error on function listen() in server.c");

	while (!finish)
	{

		int fd = -1;
		struct sockaddr_in temp;
		socklen_t len = 0;
		len = sizeof(temp);

		fd = accept(skt, (struct sockaddr *)&temp, &len);
		struct clientInfo *client = addClient(temp, serInfo->listClients);

		if (client->status == NOTCONNECTED)
		{
			pthread_mutex_lock(&client->lockInfo);
			client->clientSocket = fd;
			pthread_mutex_unlock(&client->lockInfo);

			if (pthread_create(&client->clientThread, NULL, client_thread, (void *)client) == 0)
				pthread_detach(client->clientThread);
			else
				close(fd);
		}
		else
			close(fd);
	}

	close(skt);
	return NULL;
}
