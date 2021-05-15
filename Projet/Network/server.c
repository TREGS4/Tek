#include "server.h"
#include "network_tools.h"

void printData(int type, unsigned long long len, char *buff)
{
	size_t taille = (3 + 10 + 21 + len) * 10;
	char *message = malloc(taille);
	memset(message, 0, taille);
	sprintf(message, "Type: %d\nSize: %llu\nData: ", type, len);
	size_t offset = strlen(message);

	for (size_t i = 0; i < len * 3; i += 3)
	{
		sprintf(message + offset + i, "%02x ", buff[i/3]);
	}
	message[strlen(message)] = '\n';
	message[strlen(message)] = '\n';

	printf(message);
}

void *read_thread(void *arg)
{
	struct connection *client = arg;
	unsigned long long sizeData = 0;
	int type = 0;
	char headerBuff[HEADER_SIZE];
	char *dataBuff;
	int problem = 0;
	int fdout = -1;
	size_t nbCharToRead = HEADER_SIZE;
	size_t offset = 0;
	int r = 0;

	//peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
	//comportement inconnu dans ce cas la

	while (problem == 0 && nbCharToRead)
	{
		r = read(client->socket, headerBuff + offset, nbCharToRead);
		nbCharToRead -= r;
		if (r <= 0)
			problem = 1;
	}

	memcpy(&type, headerBuff, SIZE_TYPE_MSG);
	memcpy(&sizeData, headerBuff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);
	nbCharToRead = sizeData;
	dataBuff = malloc(sizeof(char) * sizeData);

	while (problem == 0 && nbCharToRead)
	{
		r = read(client->socket, dataBuff, nbCharToRead);
		nbCharToRead -= r;
		if (r <= 0)
			problem = 1;
	}

	if (problem == 0)
	{
		if (type == 1)
		{
			fdout = client->server->fdoutIntern;
			pthread_mutex_lock(&client->server->lockReadGlobalIntern);
		}
		else
		{
			fdout = client->server->fdoutExtern;
			pthread_mutex_lock(&client->server->lockReadGlobalExtern);
		}

		write(fdout, headerBuff, HEADER_SIZE);
		write(fdout, dataBuff, sizeData);

		if (type == 1)
			pthread_mutex_unlock(&client->server->lockReadGlobalIntern);
		else
			pthread_mutex_unlock(&client->server->lockReadGlobalExtern);
	}
	else
		printf("Error while receinving data in read_thread\nError with function read or not enough bytes received\n");

	printf("Reception :\n");
	printData(type, sizeData, dataBuff);

	close(client->socket);
	free(client);
	free(dataBuff);
	return NULL;
}

/*
	This section create the server and listen for connection.
	When connection is receive, a fork is made for the client.
	Each fork contain two thread for transmit and receive data simultanously.
*/
void *Server(void *arg)
{
	struct server *server = arg;
	struct addrinfo hints;
	struct addrinfo *res;
	int connect = 0;
	int skt;
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

	while (server->status == ONLINE)
	{
		struct connection *client = malloc(sizeof(struct connection));
		pthread_t thread;
		client->server = server;

		client->socket = accept(skt, NULL, NULL);

		if (pthread_create(&thread, NULL, read_thread, (void *)client) == 0)
			pthread_detach(thread);
		else
		{
			close(client->socket);
			free(client);
		}
	}

	close(skt);
	return NULL;
}
