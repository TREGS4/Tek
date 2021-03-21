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

void finisher_client()
{
	wait(NULL);
}

void * read_thread_client(){
	printf("in read thread_client\n");
	int buff_size = 5;
	char buff[buff_size];
	int r = 1;

	while ((r = read(STDIN_FILENO, &buff, buff_size)) > 0)
		write(STDOUT_FILENO, buff, r);

	return 0;
}

void * write_thread_client(){
	printf("in write thread_client\n");
	return 0;
} 


/*
	This section create the server and listen for connection.
	When connection is receive, a fork is made for the client.
	Each fork contain two thread for transmit and receive data simultanously.
*/
int client(char** argv)
{

	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo *test;
	int connect = 0;
	int skt;
	int skt2;
	int pidc = getpid();
	pthread_t Rthr;
	pthread_t Wthr;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(argv[1], argv[2] , &hints, &res);

	while(res != NULL && connect == 0)
	{
		skt = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if(skt >= 0)
		{
			connect = 1;
			//setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(int));

			/*if (bind(skt, res->ai_addr, res->ai_addrlen) == 0)
				connect = 1;
			else
			{
				close(skt);
				res = res->ai_next;
			}	*/		
		}
		else
			res = res->ai_next;

	}

	freeaddrinfo(res);
	
	if((uint32_t)skt < 0)
		err(EXIT_FAILURE, "Error while creating the socket in client.c");
	
	/*while(pidc != 0)
	{
		signal(SIGCHLD, finisher);

		pidc = fork();
		if(pidc < 0)
			err(EXIT_FAILURE, "Error while creating the fork");

		if(pidc)
		{
			//close(skt2);
		}
		else
		{
			if(dup2(skt, STDIN_FILENO) == - 1)
				err(EXIT_FAILURE, "Error while dumping in reseau.c");
			if(dup2(skt2, STDOUT_FILENO) == - 1)
				err(EXIT_FAILURE, "Error while dumping in reseau.c");	
			if(dup2(skt2, STDERR_FILENO) == - 1)
				err(EXIT_FAILURE, "Error while dumping in reseau.c");
		
			int res1 = pthread_create(&Rthr, NULL, read_thread, NULL);
			int res2 = pthread_create(&Wthr, NULL, write_thread, NULL);
			
			if(res1 || res2 )
				err(EXIT_FAILURE, "Error while creating the threads in reseau.c");
			
			pthread_join(Wthr, NULL);
			pthread_join(Rthr, NULL);
			
			close(skt);
		}
	}*/


    //if(dup2(skt, STDIN_FILENO) == - 1)
	//	err(EXIT_FAILURE, "Error while dumping in client.c");

	
	int res1 = pthread_create(&Rthr, NULL, read_thread_client, NULL);
	int res2 = pthread_create(&Wthr, NULL, write_thread_client, NULL);
			
	if(res1 || res2 )
		err(EXIT_FAILURE, "Error while creating the threads in client.c");
			
	pthread_join(Wthr, NULL);
	pthread_join(Rthr, NULL);

	close(skt);
	exit(0);
}
