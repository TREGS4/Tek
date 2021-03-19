#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <err.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


int maint(){
return 0;}

void finisher()
{
	wait(NULL);
}

int reseau(int argc, char** argv)
{
	if (argc != 2)
		errx(EXIT_FAILURE, "Usage:\n"
				"Arg 1 = Port number (e.g. 2048)");

	struct addrinfo hints;
	struct addrinfo *res;
	int connect = 0;
	int skt;
	int skt2;
	int pidc = getpid();
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
		err(1, "Error while creating the socket");
	if(listen(skt, 5) == -1)
		err(1, "Error on function listen()");
	
	while(pidc != 0)
	{
		signal(SIGCHLD, finisher);

		skt2 = accept(skt, res->ai_addr, &res->ai_addrlen);
		if(skt2 == -1)
			err(1, "Error while connecting to the client");
		pidc = fork();
		if(pidc < 0)
			err(1, "Error while creating the fork");

		if(pidc)
		{
			close(skt2);
		}
		else
		{
			close(skt);
			if(dup2(skt2, STDIN_FILENO) == - 1)
				err(1, "Error while dumping in reseau.c");
			if(dup2(skt2, STDOUT_FILENO) == - 1)
				err(1, "Error while dumping in reseau.c");	
			if(dup2(skt2, STDERR_FILENO) == - 1)
				err(1, "Error while dumping in reseau.c");
			
			if(execlp("bc", "bc", NULL) == -1)
				err(1, "Error while running command in reseau.c");
			close(skt2);
		}
	}

	close(skt);
	exit(0);
}
