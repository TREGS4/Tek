#include "Block/block.h"
#include "Hash/sha256.h"
#include <stdio.h>

int main(){
	TRANSACTION t = {
		.sender = "adrien",
		.receiver = "paul",
		.amount = 100
	};
	BLOCK b;
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++){
		b.transactions[i] = t;
	}
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++){
		printf("txs %d: %d de %s a %s\n",i, b.transactions[i].amount, b.transactions[i].sender, b.transactions[i].receiver);
	}

	
	BYTE merkleHash[SHA256_BLOCK_SIZE];
	getMerkleHash(&b, merkleHash);
	
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++){
		printf("%02x", merkleHash[i]);
	}
	printf("\n");
}
