#include "server.h"

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

	char buffLen[SIZE_ULONGLONG + SIZE_TYPE_MSG + 1];
	char buffType[SIZE_TYPE_MSG + 1];
	char buff[BUFFER_SIZE_SOCKET];

	while (client->status == CONNECTED)
	{
		memset(buffLen, 0, SIZE_ULONGLONG + 1);
		memset(buffType, 0, SIZE_TYPE_MSG + 1);
		memset(buff, 0, BUFFER_SIZE_SOCKET);

		int r = 1;

		size_t nbToRead = SIZE_ULONGLONG + SIZE_TYPE_MSG;
		size_t nbchr = 0;

		/*Header part*/

		while (client->status != ENDED && nbToRead > 0)
		{
			if ((r = read(fdin, &buffLen + nbchr, nbToRead)) <= 0)
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

		type = atoi(&buffType[0]);
		size = (unsigned)atoll(&buffLen[SIZE_TYPE_MSG]); //not working number above 9 999 999 999

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

		if(client->status == CONNECTED)
			write(fdout, buffLen, SIZE_TYPE_MSG + SIZE_ULONGLONG);

		/*Message part*/

		while (client->status != ENDED && size > 0)
		{
			if ((r = read(fdin, &buff, nbToRead)) <= 0)
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

void *write_thread(void *arg)
{
	//printf("in write thread server\n");
	struct clientInfo *client = arg;
	int fdin = client->fdinThread;
	int fdout = client->clientSocket;

	char buff[BUFFER_SIZE_SOCKET];
	int r = 1;

	while (client->status != ENDED && (r = read(fdin, &buff, BUFFER_SIZE_SOCKET)) > 0)
		write(fdout, buff, r);

	return NULL;
}

void *client_thread(void *arg)
{
	struct clientInfo *client = arg;

	pthread_create(&client->writeThread, NULL, write_thread, arg);
	pthread_create(&client->readThread, NULL, read_thread, arg);

	pthread_join(client->readThread, NULL);
	//printf("read thread ended !\n");

	pthread_mutex_lock(&client->lockInfo);
	client->status = ENDED;
	pthread_mutex_unlock(&client->lockInfo);

	pthread_join(client->writeThread, NULL);
	//printf("write thread ended !\n");

	pthread_mutex_lock(&client->lockRead);
	pthread_mutex_lock(&client->lockWrite);

	close(client->clientSocket);

	pthread_mutex_unlock(&client->lockWrite);
	pthread_mutex_unlock(&client->lockRead);

	pthread_mutex_lock(&client->lockInfo);
	client->status = DEAD;
	pthread_mutex_unlock(&client->lockInfo);

	removeClient(client);
	printf("Client disconnected !\n");
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
			setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));

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
		struct clientInfo *client = initClient(serInfo->listClients);
		int fd = -1;
		struct sockaddr temp;
		socklen_t len = 0;
		len = sizeof(client->IPandPort.sa_data);

		fd = accept(skt, &temp, &len);
		printIP(&temp);

		pthread_mutex_lock(&client->lockInfo);
		pthread_mutex_lock(&client->lockWrite);
		pthread_mutex_lock(&client->lockRead);

		client->IPLen = len;
		client->IPandPort = temp;
		client->clientSocket = fd;

		client->status = CONNECTING;
		pthread_mutex_unlock(&client->lockRead);
		pthread_mutex_unlock(&client->lockWrite);
		pthread_mutex_unlock(&client->lockInfo);

		printf("Client connected!\n");
	}

	close(skt);
	return NULL;
}

void *connectionMaintener(void *arg)
{
	struct serverInfo *server = arg;
	struct clientInfo *client = server->listClients;
	int res = 1;

	while (res)
	{
		for (client = client->sentinel->next; client != client->sentinel; client = client->next)
		{
			if (client->status == CONNECTING)
			{
				if (pthread_create(&client->clientThread, NULL, client_thread, (void *)client) == 0)
				{
					pthread_mutex_lock(&client->lockInfo);
					client->status = CONNECTED;
					pthread_mutex_unlock(&client->lockInfo);
				}
				else
				{
					pthread_mutex_lock(&client->lockInfo);
					client->status = DEAD;
					pthread_mutex_unlock(&client->lockInfo);
					client = client->next;
					removeClient(client->prev);
				}	
			}
		}
		sleep(0.1);
	}

	return NULL;
}


int connectClient(char *IP, struct clientInfo *list)
{
	if(IP == NULL)
		return 1;

	int skt;
	struct sockaddr_in info;
	struct sockaddr *temp;
	info.sin_port = htons(6969);
	info.sin_family = AF_INET;
	inet_pton(AF_INET, IP, &info.sin_addr);
	
    if((skt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		return -1;
	}	

	if(connect(skt, (struct sockaddr *)&info, sizeof(info)) < 0)
	{
		perror(NULL);
		return -1;
	}

	temp = (struct sockaddr *)&info;
	struct clientInfo *client = initClient(list);
	pthread_mutex_lock(&client->lockInfo);
	client->clientSocket = skt;
	client->IPandPort = *temp;
	client->IPLen = sizeof(client->IPandPort.sa_data);
	client->status = CONNECTING;
	pthread_mutex_unlock(&client->lockInfo);

	return 1;
}