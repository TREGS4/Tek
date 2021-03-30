#ifndef BLOCK_H
#define BLOCK_H

#include "../Hash/sha256.h"

#define NB_TRANSACTIONS_PER_BLOCK 10

#define TRANSACTION_USER_SIZE 20
#define TRANSACTION_AMOUNT_SIZE 24
#define TRANSACTION_SIZE 2 * TRANSACTION_USER_SIZE + TRANSACTION_AMOUNT_SIZE

typedef struct
{
	char sender[TRANSACTION_USER_SIZE];
	char receiver[TRANSACTION_USER_SIZE];
	int amount;
} TRANSACTION;

typedef struct{
	BYTE previusHash[SHA256_BLOCK_SIZE];
	TRANSACTION transactions[NB_TRANSACTIONS_PER_BLOCK];
	BYTE blockHash[SHA256_BLOCK_SIZE];
} BLOCK;

void getMerkleHash(BLOCK *b, BYTE merkleHash[SHA256_BLOCK_SIZE]);
void getHash(BLOCK *b, BYTE hash[SHA256_BLOCK_SIZE]);

#endif
