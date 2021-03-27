#include "server.h"
#include "network.h"

#define PORT "6969"

struct listClientInfo *forTheFinisher;

void finisher()
{
	int wstatus;
	wait(&wstatus);
	wstatus = WEXITSTATUS(wstatus);
	removeClient(forTheFinisher->list[wstatus], forTheFinisher);
}

void *read_thread(void *args)
{
	printf("in read thread server\n");
	int *fds = args;
	int buff_size = 512;
	char buff[buff_size];
	int r = 1;

	while ((r = read(fds[0], &buff, buff_size)) > 0)
		write(fds[1], buff, r);

	return 0;
}

void *write_thread(void *args)
{
	printf("in write thread server\n");
	int *fds = args;
	int buff_size = 512;
	char buff[buff_size];
	int r = 1;

	while ((r = read(fds[0], &buff, buff_size)) > 0)
		write(fds[1], buff, r);
	return 0;
}

/*
	This section create the server and listen for connection.
	When connection is receive, a fork is made for the client.
	Each fork contain two thread for transmit and receive data simultanously.
*/
void * server(void * arg)
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
		socklen_t len = sizeof(client->IP);

		client->fd = accept(skt, client->IP, &len);
		if (client->fd == -1)
			err(EXIT_FAILURE, "Error while connecting to the client in server.c");
		
		client->status = CONNECTING;
		
		printf("Client connected!\n");
	}

	close(skt);
	return NULL;
}


void * connectionMaintener(void *arg)
{
	struct listClientInfo *clients = arg;
	forTheFinisher = clients;
	int res = 1;
	pthread_t Rthr;
	pthread_t Wthr;
	int argW[2];
	int argR[2];
	signal(SIGCHLD, finisher);

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
				}
				else
				{
					argW[0] = clients->list[i].fdinThread;
					argR[1] = clients->list[i].fdoutThread;
					argW[1] = clients->list[i].fd;
					argW[0] = clients->list[i].fd;

					pthread_create(&Wthr, NULL, write_thread, (void*) argW);
					pthread_create(&Rthr, NULL, read_thread, (void*) argR);
					pthread_join(Wthr, NULL);
					pthread_join(Rthr, NULL);

					close(clients->list[i].fd);
					close(clients->list[i].fdin);
					close(clients->list[i].fdout);
					exit(i);
				}
			}
		}
	}
	return NULL;
}