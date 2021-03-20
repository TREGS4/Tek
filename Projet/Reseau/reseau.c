#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

void finisher()
{
	wait(NULL);
}

void * read_thread(){
	int buff_size = 5;
	char buff[buff_size];
	int r = 1;

	while ((r = read(STDIN_FILENO, &buff, buff_size)) > 0)
		write(STDOUT_FILENO, buff, r);

	return 0;
}

void * write_thread(){
	printf("in write thread\n");
	return 0;
} 


/*
	This section create the server and listen for connection.
	When connection is receive, a fork is made for the client.
	Each fork contain two thread for transmit and receive data simultanously.
*/
int reseau(int argc, char** argv)
{
	if (argc != 2)
		err(EXIT_FAILURE, "Usage:\n"
				"Arg 1 = Port number (e.g. 2048)");

	struct addrinfo hints;
	struct addrinfo *res;
	int connect = 0;
	int skt;
	int skt2;
	int pidc = getpid();
	pthread_t Rthr;
	pthread_t Wthr;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, argv[1], &hints, &res);

	while(res != NULL && connect == 0)
	{
		int value = 1;
		skt = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if(skt >= 0)
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

	if(skt < 0)
		err(EXIT_FAILURE, "Error while creating the socket");
	if(listen(skt, 5) == -1)
		err(EXIT_FAILURE, "Error on function listen()");
	
	while(pidc != 0)
	{
		signal(SIGCHLD, finisher);

		skt2 = accept(skt, res->ai_addr, &res->ai_addrlen);
		if(skt2 == -1)
			err(EXIT_FAILURE, "Error while connecting to the client");
		pidc = fork();
		if(pidc < 0)
			err(EXIT_FAILURE, "Error while creating the fork");

		if(pidc)
		{
			close(skt2);
		}
		else
		{
			close(skt);
			if(dup2(skt2, STDIN_FILENO) == - 1)
				err(EXIT_FAILURE, "Error while dumping in reseau.c");
			/*if(dup2(skt2, STDOUT_FILENO) == - 1)
				err(EXIT_FAILURE, "Error while dumping in reseau.c");	
			if(dup2(skt2, STDERR_FILENO) == - 1)
				err(EXIT_FAILURE, "Error while dumping in reseau.c");*/
		
			int res1 = pthread_create(&Rthr, NULL, read_thread, NULL);
			int res2 = pthread_create(&Wthr, NULL, write_thread, NULL);
			
			if(res1 || res2 )
				err(EXIT_FAILURE, "Error while creating the threads in reseau.c");
			
			pthread_join(Wthr, NULL);
			pthread_join(Rthr, NULL);
			
			close(skt2);
		}
	}

	close(skt);
	exit(0);
}
