#include "server.h"
#include "network.h"

void finisher()
{
	wait(NULL);
}

void ender()
{

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
		if(buff[r - 1] == '\0')
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
		if(buff[r - 1] == '\0')
			end = 1;
	}	

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
		pthread_mutex_lock(&clients->lockList);
		client->IPLen = sizeof(client->IP.sa_data);

		client->fd = accept(skt, &client->IP, &client->IPLen);
		printIP(&client->IP);

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
	int res = 1;
	pthread_t Rthr;
	pthread_t Wthr;
	struct sigaction IsEnded;
	memset (&IsEnded, '\0', sizeof(IsEnded));
	signal(SIGCHLD, finisher);
	signal(jsp, )

	while (res)
	{
		for (size_t i = 0; i < clients->size && res; i++)
		{
			if (clients->list[i].status == CONNECTING)
			{
				res = fork();
				if (res < 0)
					err(EXIT_FAILURE, "Error while forking the client");

				if (res > 0)
				{
					clients->list[i].status = CONNECTED;
					close(clients->list[i].fd);
					close(clients->list[i].fdinThread);
					close(clients->list[i].fdoutThread);
				}
				else
				{
					pthread_create(&Wthr, NULL, write_thread, (void *)&clients->list[i]);
					pthread_create(&Rthr, NULL, read_thread, (void *)&clients->list[i]);

					pthread_join(Rthr, NULL);
					printf("read thread ended !\n");
					clients->list[i].status = ENDED;

					pthread_join(Wthr, NULL);
					printf("write thread ended !\n");

					pthread_mutex_lock(&clients->lockRead);
					close(clients->fdin);
					close(clients->list[i].fdinThread);
					pthread_mutex_unlock(&clients->lockRead);

					pthread_mutex_lock(&clients->lockWrite);
					close(clients->fdout);
					close(clients->list[i].fdoutThread);
					pthread_mutex_unlock(&clients->lockWrite);
					
					close(clients->list[i].fdout);
					close(clients->list[i].fd);
					
					

					//removeClient(clients->list[i], clients);
					printf("Client disconnected !\n");
					exit(EXIT_SUCCESS);
				}
			}
		}
	}

	return NULL;
}