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


void printTransaction(TRANSACTION t) 
{
	char * time_str = ctime(&t.time);
    time_str[strlen(time_str)-1] = '\0';
	printf("Amount : %14ld$. From %s to %s. date: %s\n", t.amount, t.sender, t.receiver, time_str);
}

void printBlock(BLOCK block)
{
	printf("prevHash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", block.previusHash[i]);
	}
	printf("\n");
	
	printf("currHash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", block.blockHash[i]);
	}
	printf("\n");
	
	size_t nbTxs = block.tl.size;
	for (size_t i = 0; i < nbTxs; i++)
	{
		printTransaction(block.tl.transactions[i]);
	}	
}

void printBlockchain(BLOCKCHAIN blockchain)
{
	printf("-------------------BLOCKCHAIN-------------------\n\n");
	printf("----Genesis----\n");
	
	printf("Hash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", blockchain.blocks[0].blockHash[i]);
	}
	printf("\n");
	printf("---------------\n\n");
	
	for(size_t i = 1; i < blockchain.blocksNumber; i++)
	{
		printf("---------------BLOCK %02ld---------------\n", i);
		printBlock(blockchain.blocks[i]);
		printf("--------------------------------------\n\n");
	}
	printf("------------------------------------------------\n");
}




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

	struct server *server = initServer();
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

	while (server->status != ONLINE){
		sleep(0.2);
	}
	//pthread_t gestionthread;
	//pthread_create(&gestionthread, NULL, gestion, (void *)server);

	char *data = "salope";
	MESSAGE *msg = CreateMessage(2, strlen(data), data);
	printf("message push : %p\n", msg);
	shared_queue_push(server->OutgoingMessages, msg);

	sleep(5);
	printf("Wainting for stop\n");
	server->status = EXITING;
	pthread_join(networkthread, NULL);
	//pthread_join(gestionthread, NULL);
	
	printf("Network is ended\n");
	freeServer(network.server);
	printf("Server is freed\n");

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	/*TRANSACTION t = CreateTxs(104, "Adrien", "Tom");
	free(t.sender);
	free(t.receiver);
	
	TRANSACTION t2 = 
	{
		.sender = "Thimot",
		.receiver = "Margaux",
		.amount = 358
	};
	
	TRANSACTION t3 = 
	{
		.sender = "Pierre",
		.receiver = "Hugo",
		.amount = 226
	};
	
	BLOCK b;
	b.tl = initListTxs();
	for (int i = 0; i < 3; i++)
	{
		t.amount = rand() % 3000 + 1;
		addTx(&b.tl, &t);
	}

	BLOCK b2;
	b2.tl = initListTxs();
	for (int i = 0; i < 5; i++)
	{
		t2.amount = rand() % 3000 + 1;
		addTx(&b2.tl, &t2);
	}
	
	BLOCK b3;
	b3.tl = initListTxs();
	for (int i = 0; i < 1; i++)
	{
		t3.amount = rand() % 3000 + 1;
		addTx(&b3.tl, &t3);
	}

	BLOCKCHAIN bc = initBlockchain();
	addBlock(&bc, b);
	addBlock(&bc, b2);
	addBlock(&bc, b3);
	printf("Number of blocks in the blockchain : %lu.\n\n", bc.blocksNumber);
	printBlockchain(bc);
	printf("\n");


	BLOCKCHAIN_BIN txsbin = blockchainToBin(&bc);
	for (size_t i = 0; i < txsbin.nbBytes; i++)
	{
		if (i % 20 == 0){
			printf("\n");
		}
		printf("%02x ", txsbin.bin[i]);
	}
	printf("\n");

	


	freeBlockchain(&bc);


	time_t rawtime;
	time (&rawtime);

	//printf ( "Current local time and date: %ld", sizeof(rawtime) );
*/
	grosTest(argc, argv);
	
}