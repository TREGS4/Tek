#include "blockchain.h"
#include "block.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


BLOCK *getLastBlock(BLOCKCHAIN *blockchain)
{
	return &(blockchain->blocks[blockchain->blocksNumber - 1]);
}

void addBlock(BLOCKCHAIN *blockchain, BLOCK block)
{
	BLOCK lastBlock = *(getLastBlock(blockchain));
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		block.previusHash[i] = lastBlock.blockHash[i];
	}
	
	BYTE hash[SHA256_BLOCK_SIZE];
	getHash(&block, hash);
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		block.blockHash[i] = hash[i];
	}
	
	blockchain->blocksNumber += 1;
	
	blockchain->blocks = realloc(blockchain->blocks, blockchain->blocksNumber * sizeof(BLOCK));
	if (blockchain->blocks == NULL)
		exit(1);
	
	blockchain->blocks[blockchain->blocksNumber - 1] = block;	
	
}

BLOCKCHAIN initBlockchain()
{
	BLOCK newGenesis = createGenesis();
	
	BLOCKCHAIN newBlockchain =  
	{
		.blocks = NULL,
		.blocksNumber = 1,
	};

	newBlockchain.blocks = malloc(sizeof(BLOCK));
	if (newBlockchain.blocks == NULL)
		exit(1);
		
	newBlockchain.blocks[0] = newGenesis;

	return newBlockchain;	
}

BLOCK createGenesis()
{
	BLOCK newGenesis;
	
	BYTE buf[] = "HarryStylesAuraitDuAvoirUnGrammyPourHS1";
	sha256(buf, newGenesis.blockHash);
	
	return newGenesis;
}
