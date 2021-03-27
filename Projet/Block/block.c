#include "block.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>



void txsToString(TRANSACTION *txs, char buf[TRANSACTION_SIZE])
{
	sprintf(buf, "%s%s%014d", txs->sender, txs->receiver, txs->amount);
}

void getMerkleHash(BLOCK *b, BYTE merkleHash[SHA256_BLOCK_SIZE])
{
	BYTE buf[TRANSACTION_SIZE*NB_TRANSACTIONS_PER_BLOCK];
	size_t offset = 0;
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++){
		char txs_buf[TRANSACTION_SIZE];
		txsToString(&(b->transactions[i]), txs_buf);
		sprintf((char*)buf+offset, "%s", txs_buf);
		offset += strlen(txs_buf);
	}
	sha256(buf, merkleHash);
}
