#ifndef BLOCK_H
#define BLOCK_H

#include "../Hash/sha256.h"
#include "transactions.h"


typedef struct
{
	BYTE previusHash[SHA256_BLOCK_SIZE];
	TRANSACTIONS_LIST tl;
	BYTE blockHash[SHA256_BLOCK_SIZE];
} BLOCK;

void getMerkleHash(BLOCK *b, BYTE merkleHash[SHA256_BLOCK_SIZE]);
void getHash(BLOCK *b, BYTE hash[SHA256_BLOCK_SIZE]);
char *blockToJson(BLOCK *b);

#endif
