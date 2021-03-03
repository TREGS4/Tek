#ifndef BLOCK_H
#define BLOCK_H

#include "../Hash/sha256.h"
#include "transaction.h"

typedef struct{
	BYTE previusHash[SHA256_BLOCK_SIZE];
	TRANSACTION transactions[10];
	BYTE blockHash[SHA256_BLOCK_SIZE];
} BLOCK;

void calculateHash(BLOCK b, BYTE hash[SHA256_BLOCK_SIZE]);

#endif
