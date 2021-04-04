#include "server.h"
#include "network.h"

struct clientInfoThread
{
	struct listClientInfo *clients;
	size_t pos;
} 

void *read_thread(void *arg)
{
	printf("in read thread server\n");
	struct clientInfo *client = arg;
	int fdin = client->fd;
	int fdout = client->fdoutThread;
	int end = 0;

	char buff[BUFFER_SIZE_SOCKET];
	int r = 1;

	while (!end && client->status != ENDED && (r = read(fdin, &buff, BUFFER_SIZE_SOCKET)) > 0)
	{
		write(fdout, buff, r);
		if (buff[r - 1] == '\0')
			end = 1;
	}

	return NULL;
}

void *write_thread(void *arg)
{
	printf("in write thread server\n");
	struct clientInfo *client = arg;
	int fdin = client->fdinThread;
	int fdout = client->fd;
	int end = 0;

	char buff[BUFFER_SIZE_SOCKET];
	int r = 1;

	while (!end && client->status != ENDED && (r = read(fdin, &buff, BUFFER_SIZE_SOCKET)) > 0)
	{
		write(fdout, buff, r);
		if (buff[r - 1] == '\0')
			end = 1;
	}

	return NULL;
}

void *client_thread(void *arg)
{
	struct clientInfoThread *clientInfo = arg;
	size_t pos = clientInfo->pos;
	struct clientInfo *client = clientInfo->clients->list[];
	pthread_mutex_unlock(&clientInfo->clients->lockGetPos);
	pthread_t Wthr, Rthr;

	pthread_create(&Wthr, NULL, write_thread, arg);
	pthread_create(&Rthr, NULL, read_thread, arg);

	pthread_join(Rthr, NULL);
	printf("read thread ended !\n");
	client->status = ENDED;
	pthread_mutex_lock(client->lockList);
	
	pthread_mutex_unlock(client->lockList);

	if(client->status == ENDED)
		printf("here\n");
	pthread_join(Wthr, NULL);
	printf("write thread ended !\n");

	pthread_mutex_lock(client->lockRead);
	pthread_mutex_lock(client->lockWrite);

	close(client->fdinThread);
	close(client->fdoutThread);
	close(client->fdout);
	close(client->fd);

	pthread_mutex_unlock(client->lockWrite);
	pthread_mutex_unlock(client->lockRead);
	

	//removeClient(clients->list[i], clients);
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
	struct listClientInfo *clients = arg;
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
		struct clientInfo *client = initClient(clients);
		int fd = -1;
		struct sockaddr temp;
		socklen_t len = 0;
		len = sizeof(client->IP.sa_data);

		fd = accept(skt, &temp, &len);
		printIP(&temp);

		pthread_mutex_lock(&clients->lockList);
		client->IPLen = len;
		client->IP = temp;
		client->fd = fd;

		if (client->fd == -1)
			err(EXIT_FAILURE, "Error while connecting to the client in server.c");

		client->status = CONNECTING;
		pthread_mutex_unlock(&clients->lockList);

		printf("Client connected!\n");
	}

	close(skt);
	return NULL;
}

void *connectionMaintener(void *arg)
{
	struct listClientInfo *clients = arg;
	size_t listThreadSize = clients->size;
	pthread_t *listThread = malloc(listThreadSize * sizeof(pthread_t));
	int res = 1;

	while (res)
	{
		if (listThreadSize < clients->size)
		{
			listThreadSize = clients->size;
			listThread = realloc(listThread, listThreadSize * sizeof(pthread_t));
			if (listThread == NULL)
				err(EXIT_FAILURE, "Error while reallocating thread list in server.c");
		}

		for (size_t i = 0; i < clients->size && res; i++)
		{
			if (clients->list[i].status == CONNECTING)
			{
				if(pthread_create(&listThread[i], NULL, client_thread, (void *)&clients->list[i]) == 0)
				{
					pthread_mutex_lock(&clients->lockList);
					clients->list[i].status = CONNECTED;
					pthread_mutex_unlock(&clients->lockList);
				}
			}
		}
	}

	for (size_t i = 0; i < listThreadSize; i++)
		pthread_join(listThread[i], NULL);

	return NULL;
}