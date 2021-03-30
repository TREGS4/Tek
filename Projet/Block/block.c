#include "block.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void txsToString(TRANSACTION *txs, char buf[TRANSACTION_SIZE])
{
	sprintf(buf, "%s%s%014d", txs->sender, txs->receiver, txs->amount);
}

void getHash(BLOCK *b, BYTE hash[SHA256_BLOCK_SIZE])
{
	BYTE buf[4 * SHA256_BLOCK_SIZE + 1];
	BYTE merkleHash[SHA256_BLOCK_SIZE];
	getMerkleHash(b, merkleHash);
	
	char var1[SHA256_BLOCK_SIZE*2];
	char var2[SHA256_BLOCK_SIZE*2];
	sha256ToAscii(b->previusHash, var1);
	sha256ToAscii(merkleHash, var2);
	
	memcpy(buf, var1, SHA256_BLOCK_SIZE*2);
	memcpy(buf + SHA256_BLOCK_SIZE *2, var2, SHA256_BLOCK_SIZE*2);
	buf[SHA256_BLOCK_SIZE * 4] = 0;
	
	sha256(buf, hash);
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
