#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include "API/API.h"
#include "Network/network.h"
#include <signal.h>

void *printtest(void *arg)
{
	struct server *server = arg;

	while (1)
	{
		sleep(2);
		ServerListToJSON(server);
	}
	
	

	return NULL;
}

int main(int argc, char *argv[])
{
	if(argc > 5)
		return 0;

	struct server *server = initServer();
	pthread_t t;

	pthread_create(&t, NULL, printtest, (void *)server);

	Network(server, argv[1], argv[2], argv[3], argv[4]);
	return 0;
}
