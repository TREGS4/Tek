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

	char str[9];
	char type = 69;
	unsigned long long size = strlen(data);

	memcpy(str, &type, 1);
	memcpy(str + 1, &size, 8);

	while (1)
	{
		sleep(2);

		pthread_mutex_lock(&tst.mutext);
		write(tst.fd[1], str, 10);
		write(tst.fd[1], data, size);
		pthread_mutex_unlock(&tst.mutext);
		printf("Message send\n");
	}

	pthread_join(thread, NULL);
	return 0;
}