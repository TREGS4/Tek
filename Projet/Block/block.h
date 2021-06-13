#ifndef BLOCK_H
#define BLOCK_H

#include "../Hash/sha256.h"
#include "transactions.h"
#include "../Noeud_de_minage/minage.h"

#include <stdbool.h>
#include "../general_informations.h"


typedef struct
{
	BYTE previusHash[SHA256_BLOCK_SIZE];
	TRANSACTIONS_LIST tl;
	BYTE blockHash[SHA256_BLOCK_SIZE];
	ull_t proof;
	time_t time;
} BLOCK;

typedef struct
{
	BYTE *bin;
	ull_t nbBytes;
} BLOCK_BIN;

BLOCK initBlock();
void freeBlock(BLOCK *b);
void getMerkleHash(BLOCK *b, BYTE merkleHash[SHA256_BLOCK_SIZE]);
void getHash(BLOCK *b, BYTE hash[SHA256_BLOCK_SIZE]);
char *blockToJson(BLOCK *b);
BLOCK_BIN blockToBin(BLOCK *b);
BLOCK binToBlock(BYTE *bin);
ull_t getSizeOf_blockbin(BLOCK *b);
bool isGenesis(BLOCK *b);
void printBlock(BLOCK block);

#endif
