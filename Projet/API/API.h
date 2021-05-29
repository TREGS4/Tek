#ifndef API_H
#define API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <err.h>
#include <semaphore.h>
#include "../Network/network_tools.h"
#include "../Block/transactions.h"
#include "../Block/blockchain.h"



typedef struct{ 
	BLOCKCHAIN_M *bc_m;
	TL_M *tl_m;
	struct server *server;
	shared_queue *outgoingTxs;
} API_THREAD_ARG;

struct WORK_ARG { 
	int client_socket_id;
	BLOCKCHAIN_M *bc_m;
	struct server *server;
	TL_M *tl_m;
	shared_queue *outgoingTxs;
};
void *API(void *args);


#endif