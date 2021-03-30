#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include "../Hash/sha256.h"
#include "block.h"


typedef struct
{
	BLOCK *blocks;
	size_t blocksNumber;
	
} BLOCKCHAIN;

BLOCK *getLastBlock(BLOCKCHAIN *blockchain);
void addBlock(BLOCKCHAIN *blockchain, BLOCK block);
BLOCKCHAIN initBlockchain();
BLOCK createGenesis();

#endif
