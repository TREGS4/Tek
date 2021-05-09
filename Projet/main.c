#include "./Network/network.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>


struct test
{
	int fd[2];
	pthread_mutex_t mutext;
	char *IP;
	char *firstserver;
};

void *moncul(void *arg)
{
	struct test *tst = arg;

	network(&tst->fd[0], &tst->fd[1], &tst->mutext, tst->IP, tst->firstserver);

	return NULL;
}

int main(int argc, char **argv)
{
	if (argc > 3)
		return -1;

	struct test tst;
	tst.fd[1] = -1;
	tst.fd[0] = -1;
	tst.IP = argv[1];
	tst.firstserver = argv[2];

	pthread_t thread;
	pthread_create(&thread, NULL, moncul, (void *)&tst);

	char *data = "Hello world !\n";
	
	while (1)
	{
		sleep(7);

		pthread_mutex_lock(&tst.mutext);
		
		SendMessage(data, tst.fd[1]);

		pthread_mutex_unlock(&tst.mutext);
		printf("Message send\n");
	}

	pthread_join(thread, NULL);
	return 0;
}