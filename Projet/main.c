#include "./Network/network.h"

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
	if(argc > 3)
		return -1;

	struct test tst;
	tst.IP = argv[1];
	tst.firstserver = argv[2];

	pthread_t thread;
	pthread_create(&thread, NULL, moncul, (void *)&tst);
	

	char str[9 + 1];

	char type = 69;
	unsigned long long size = 1;

	memcpy(str, &type, 1);
	memcpy(str + 1, &size, 8);
	
	str[9] = 'H';

	write(tst.fd[1], str, 10); 

	
	return 0;

}
