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





struct WORK_ARG { 
	int client_socket_id;
	 BLOCKCHAIN* block_list;
	 struct server *server_list;
	 TRANSACTIONS_LIST* transaction_list;
} ;
int API(BLOCKCHAIN* block_list, struct server* server_list ,TRANSACTIONS_LIST* transaction_list  );


#endif