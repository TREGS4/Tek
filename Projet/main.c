#include "Block/block.h"
#include "Block/transactions.h"
#include "Block/blockchain.h"
#include "Hash/sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include "API/API.h"
#include "Network/network.h"
#include <signal.h>


int main(){
	struct server *server_list = initServer();
	TRANSACTIONS_LIST tl = initListTxs();
	BLOCKCHAIN block_list = initBlockchain();
	API(&block_list, server_list ,&tl);
	return 0;
}
