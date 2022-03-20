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

#define DEFAULT_DIFFICULTY 3
#define DEFAULT_API_STATUS FALSE
#define DEFAULT_MINING_STATUS FALSE
#define DEFAULT_NB_MINING_THREAD ((sysconf(_SC_NPROCESSORS_CONF) - 1) == 0 ? 1 : (sysconf(_SC_NPROCESSORS_CONF) - 1))
#define MANUAL "Syntax: tek [OPTIONS] -ip YOUR IP OR HOSTNAME [OPTIONS]\n\n"\
"  -p		Port of the node, set to DEFAULT_PORT by default\n"\
"  -ip2 		IP or hostname of another node in the network.\n"\
"  -p2 		Port of the other node, set to DEFAULT_PORT by default.\n"\
"  -a 		Active the API part of the node. Disabled by default.\n"\
"  -pa      Port of the api server. Set to DEFAULT_API_PORT by default. \n"\
"  -m 		Active the mining part of the node. Disabled by default.\n"\
"  -d 		The difficulty for the mining, set to the DEFAULT_DIFFICULTY by default.\n"\
"		This parameter is for tests only. Normaly this information is given by the network.\n"\
"  -l		Active the loading of the binary file of the blockchain. path: 'bcsave.data'.\n"\
"  -nbthr	The number of thread the mining thread can use, set to the number of cores - 1 by default.\n"\


typedef struct
{
	char *IP;
	char *port;
	char *firstserverIP;
	char *portFirstServer;
	struct server *server;
} NETWORK;


typedef struct
{
	NETWORK network;
	int api;
	char *api_port;
	int mining;
	int difficulty;
	int nb_mining_thread;
	int load_blockchain;
} ARGS;

void *NetworkThread(void *arg)
{
	NETWORK *network = arg;
	Network(network->server, network->IP, network->port, network->firstserverIP, network->portFirstServer);

	return NULL;
}

void *GestionThread(void *arg)
{
	ARGS *args = arg;
	gestion(args->api, args->mining, args->difficulty, args->nb_mining_thread, args->network.server, args->load_blockchain, args->api_port);

	return NULL;
}

int LaunchTEK(ARGS *args)
{
	args->network.server = initServer(args->api, args->mining);

	pthread_t networkthread;
	pthread_t gestionthread;

	
	pthread_create(&networkthread, NULL, NetworkThread, (void *)&args->network);

	while (args->network.server->status != ONLINE)
	{
		sleep(0.1);
	}

	pthread_create(&gestionthread, NULL, GestionThread, (void *)args);

	pthread_join(networkthread, NULL);
	pthread_join(gestionthread, NULL);

	freeServer(args->network.server);

	return EXIT_SUCCESS;
}

int ProcessingArgs(size_t argc, char **argv, ARGS *args)
{
	size_t i = 1;
	int correct = TRUE;

	while (i < argc && correct)
	{
		if (memcmp(argv[i], "-a", 3) == 0)
		{
			args->api = TRUE;
			i++;
		}
		else if (memcmp(argv[i], "-m", 3) == 0)
		{
			args->mining = TRUE;
			i++;
		}
		else if (memcmp(argv[i], "-l", 3) == 0)
		{
			args->load_blockchain = TRUE;
			i++;
		}
		else if (argv[i][0] == '-' && i < argc - 1)
		{
			if (memcmp(argv[i], "-ip", 4) == 0)
			{
				size_t len = strlen(argv[++i]) + 1;
				args->network.IP = malloc(sizeof(char) * len);
				memcpy(args->network.IP, argv[i++], len);
			}
			else if (memcmp(argv[i], "-p", 3) == 0)
			{
				size_t len = strlen(argv[++i]) + 1;
				args->network.port = malloc(sizeof(char) * len);
				memcpy(args->network.port, argv[i++], len);
				if(atoi(args->network.port) <= 0)
					correct = FALSE;
			}
			else if (memcmp(argv[i], "-ip2", 5) == 0)
			{
				size_t len = strlen(argv[++i]) + 1;
				args->network.firstserverIP = malloc(sizeof(char) * len);
				memcpy(args->network.firstserverIP, argv[i++], len);
			}
			else if (memcmp(argv[i], "-p2", 4) == 0)
			{
				size_t len = strlen(argv[++i]) + 1;
				args->network.portFirstServer = malloc(sizeof(char) * len);
				memcpy(args->network.portFirstServer, argv[i++], len);
				if(atoi(args->network.portFirstServer) <= 0)
					correct = FALSE;
			}
			else if (memcmp(argv[i], "-pa", 4) == 0)
			{
				size_t len = strlen(argv[++i]) + 1;
				free(args->api_port);
				args->api_port = malloc(sizeof(char) * len);
				memcpy(args->api_port, argv[i++], len);
				if(atoi(args->api_port) <= 0)
					correct = FALSE;
			}
			else if (memcmp(argv[i], "-d", 3) == 0)
			{
				args->difficulty = atoi(argv[++i]);
				i++;
				if(args->difficulty <= 0)
					correct = FALSE;
			}
			else if (memcmp(argv[i], "-nbthr", 7) == 0)
			{
				args->nb_mining_thread = atoi(argv[++i]);
				i++;
				if(args->nb_mining_thread <= 0)
					correct = FALSE;
			}
			else
			{
				correct = FALSE;
			}
		}
		else
		{
			correct = FALSE;
		}
	}

	if (args->network.IP == NULL)
	{
		correct = FALSE;
	}

	return correct;
}

ARGS *initArgs()
{
	ARGS *args = malloc(sizeof(ARGS));
	if (args != NULL)
	{
		args->network.IP = NULL;
		args->network.port = NULL;
		args->network.firstserverIP = NULL;
		args->network.portFirstServer = NULL;

		args->api = DEFAULT_API_STATUS;
		args->api_port = strdup(DEFAULT_API_PORT);
		args->mining = DEFAULT_MINING_STATUS;
		args->difficulty = DEFAULT_DIFFICULTY;
		args->nb_mining_thread = DEFAULT_NB_MINING_THREAD;
		args->load_blockchain = FALSE;
	}

	return args;
}

int main(int argc, char **argv)
{
	ARGS *args = initArgs();

	if (args == NULL)
	{
		fprintf(stderr, "Error while alocation memory in main.c\n");
		return EXIT_FAILURE;
	}

	if(argc == 1 || memcmp(argv[1], "-h", 2) == 0 || memcmp(argv[1], "h", 1) == 0)
	{
		printf("%s\n", MANUAL);
	}
	else if (ProcessingArgs((size_t)argc, argv, args) == TRUE)
	{
		LaunchTEK(args);
	}
	else
	{
		fprintf(stderr, "Error while giving argument\n\n%s\n", MANUAL);
	}

	if (args->network.IP != NULL)
		free(args->network.IP);
	if (args->network.port != NULL)
		free(args->network.port);
	if (args->network.firstserverIP != NULL)
		free(args->network.firstserverIP);
	if (args->network.portFirstServer != NULL)
		free(args->network.portFirstServer);
	free(args);
}
