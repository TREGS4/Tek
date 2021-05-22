#include "server.h"
#include "network_tools.h"

void *read_thread(void *arg)
{
	struct connection *client = arg;
	unsigned long long sizeData = 0;
	int type = 0;
	char headerBuff[HEADER_SIZE];
	char *dataBuff;
	int problem = 0;
	int ended = 0;
	size_t nbCharToRead = HEADER_SIZE;
	size_t offset = 0;
	int r;

	//peut y avoir un souci si la taille de data depasse la taille du buffer du file descriptor
	//comportement inconnu dans ce cas la

	while (problem == 0 && nbCharToRead)
	{
		r = read(client->socket, headerBuff + offset, nbCharToRead);
		nbCharToRead -= r;
		if (r <= 0)
		{
			problem = 1;
			if (r == 0)
				ended = 1;
		}	
		else
			offset += r;
	}

	if (problem == 0)
	{
		memcpy(&type, headerBuff, SIZE_TYPE_MSG);
		memcpy(&sizeData, headerBuff + SIZE_TYPE_MSG, SIZE_DATA_LEN_HEADER);
		nbCharToRead = sizeData;
		dataBuff = malloc(sizeof(char) * (sizeData + HEADER_SIZE));
		memcpy(dataBuff, headerBuff, HEADER_SIZE);
	}

	offset = 0;
	while (problem == 0 && nbCharToRead)
	{
		r = read(client->socket, dataBuff + HEADER_SIZE + offset, nbCharToRead);
		nbCharToRead -= r;
		if (r <= 0)
			problem = 1;
		else
			offset += r;
	}

	if (problem == 0)
	{
		MESSAGE message = BinToMessage(dataBuff);

		switch (type)
		{
		case TYPE_NETWORK:
			addServerFromMessage(message, client->server);
			DestroyMessage(message);
			break;
		default:
			shared_queue_push(client->server->IncomingMessages, message);
			break;
		}
	}
	else if(ended == 0)
		printf("Error while receinving data in read_thread\nError with function read or not enough bytes received\n");

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

	getaddrinfo(NULL, server->address.port, &hints, &res);

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

	pthread_mutex_lock(&server->lockStatus);
	server->status = ONLINE;
	pthread_mutex_unlock(&server->lockStatus);

	while (server->status != EXITING)
	{
		struct connection *client = malloc(sizeof(struct connection));
		pthread_t thread;
		client->server = server;
		socklen_t temp = sizeof(client->IP);

		client->socket = accept(skt, (struct sockaddr *)&client->IP, &temp);

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
