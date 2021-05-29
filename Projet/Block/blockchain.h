#ifndef BLOCKCHAIN_H
#define BLOCKCHAIN_H
#include "../Hash/sha256.h"
#include "block.h"
#include "../general_informations.h"

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

typedef struct {
    BLOCKCHAIN bc;
    pthread_mutex_t mutex;
} BLOCKCHAIN_M;


BLOCK *getLastBlock(BLOCKCHAIN *blockchain);
void addBlock(BLOCKCHAIN *blockchain, BLOCK block);
BLOCKCHAIN initBlockchain();
BLOCK createGenesis();
int checkBlockchain(BLOCKCHAIN *blockchain);
char *blockchainToJson(BLOCKCHAIN *bc);
BLOCKCHAIN_BIN blockchainToBin(BLOCKCHAIN *bc);
BLOCKCHAIN binToBlockchain(BYTE *bin);
void freeBlockchain(BLOCKCHAIN *bc);
size_t amountMoney(char *address, BLOCKCHAIN *bc);
int enoughMoney(char *address, size_t amount, BLOCKCHAIN *bc);

#endif
