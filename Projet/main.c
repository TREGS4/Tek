#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include "API/API.h"
#include "Network/network.h"
#include <signal.h>

int main()
{

	BLOCKCHAIN_M bc_m;
	pthread_mutex_init(&bc_m.mutex, NULL);
	bc_m.bc = initBlockchain();

	TL_M txs_temp_m;
	pthread_mutex_init(&txs_temp_m.mutex, NULL);
	txs_temp_m.tl = initListTxs();

	pthread_t api_thread;
	shared_queue *api_txs;

	api_txs = shared_queue_new();
	API_THREAD_ARG args = {
		.server = initServer(),
		.bc_m = &bc_m,
		.tl_m = &txs_temp_m,
		.outgoingTxs = api_txs};

	pthread_create(&api_thread, NULL, API, (void *)&args);

	TRANSACTION temp = CreateTxs(10, "ma bite", "mon couteau");

	for(size_t i = 0; i < 1000; i++)
	{
		addTx(&args.tl_m->tl, &temp);
	}
	

	pthread_join(api_thread, NULL);

	free(temp.receiver);
	free(temp.sender);
	free(txs_temp_m.tl.transactions);
	while (!shared_queue_isEmpty(api_txs))
	{
		TRANSACTION *temp2 = shared_queue_pop(api_txs);
		free(temp2->sender);
		free(temp2->receiver);
		free(temp2);
	}
	shared_queue_destroy(api_txs);
	freeBlockchain(&bc_m.bc);

	
	freeServer(args.server);
	return 0;
}