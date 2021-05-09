#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include "../Hash/sha256.h"
#include "block.h"


typedef struct
{
	BLOCK *blocks;
	size_t blocksNumber;
	
} BLOCKCHAIN;

typedef struct
{
	BYTE *bin;
	size_t nbBytes;
} BLOCKCHAIN_BIN;

BLOCK *getLastBlock(BLOCKCHAIN *blockchain);
void addBlock(BLOCKCHAIN *blockchain, BLOCK block);
BLOCKCHAIN initBlockchain();
BLOCK createGenesis();
int checkBlockchain(BLOCKCHAIN *blockchain);
char *blockchainToJson(BLOCKCHAIN *bc);
BLOCKCHAIN_BIN blockchainToBin(BLOCKCHAIN *bc);

#endif
