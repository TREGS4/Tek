#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include <netinet/in.h>


#ifndef   NI_MAXHOST
#define   NI_MAXHOST 1025
#endif


int reseau()
{
	int fd[2];
	int fd2[2];

	if(pipe(fd))
		err(1, "error while creating pipe");

	if(fork())
	{
		close(fd[0]);
		close(fd2[1]);

		close(fd[1]);
		close(fd2[0]);




	}
	else
	{
		close(fd[1]);
		close(fd2[0]);

		dup2(fd[0], STDIN_FILENO);


		close(fd[0]);
		close(fd2[1]);
		struct addrinfo *result;
		struct addrinfo *res;
		int error;

		/* resolve the domain name into a list of addresses */
		error = getaddrinfo("www.example.com", NULL, NULL, &result);
		if (error != 0)
		{   
			fprintf(stderr, "error in getaddrinfo:\n");
			return EXIT_FAILURE;
		}   

		/* loop over all returned results and do inverse lookup */
		for (res = result; res != NULL; res = res->ai_next)
		{   
			char hostname[NI_MAXHOST] = "";

			error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0); 
			if (error != 0)
			{
				fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
				continue;
			}
			if (*hostname != '\0')
				printf("hostname: %s\n", hostname);
		}   

		freeaddrinfo(result);
	}


	wait(NULL);
	return 0;
}

