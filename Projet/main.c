#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include "Noeud_de_gestion/gestion.h"
#include <stdio.h>
#include <stdlib.h>
#include "API/API.h"
#include "Network/network.h"
#include <time.h>

typedef struct
{
	char *IP;
	char *port;
	char *firstserverIP;
	char *portFirstServer;
	struct server *server;
} NETWORK;

void *NetworkThread(void *arg)
{
	NETWORK *network = arg;
	Network(network->server, network->IP, network->port, network->firstserverIP, network->portFirstServer);

	return NULL;
}

int grosTest(int argc, char **argv)
{
	if (argc > 5 || argc < 2)
	{
		printf("Too few or to many arguments\n");
		return -1;
	}

	struct server *server = initServer(TRUE, TRUE);
	NETWORK network;
	network.server = server;
	network.IP = argv[1];
	if (argc > 2)
		network.port = argv[2];
	else
		network.port = NULL;
	if (argc > 3)
		network.firstserverIP = argv[3];
	else
		network.firstserverIP = NULL;
	if (argc > 4)
		network.portFirstServer = argv[4];
	else
		network.portFirstServer = NULL;

	pthread_t networkthread;
	pthread_create(&networkthread, NULL, NetworkThread, (void *)&network);

	while (server->status != ONLINE)
	{
		sleep(0.2);
	}
	pthread_t gestionthread;
	pthread_create(&gestionthread, NULL, gestion, (void *)server);

	//sleep(10);
	//server->status = EXITING;

	pthread_join(networkthread, NULL);
	pthread_join(gestionthread, NULL);

	freeServer(network.server);

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	grosTest(argc, argv);
}