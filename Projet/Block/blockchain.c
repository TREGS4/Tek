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
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++){
		newGenesis.previusHash[i] = 0;
	}
	
	return newGenesis;
}



char *blockchainToJson(BLOCKCHAIN *bc){
	char *s1 = "{\"blocks\":[";
	char *s2 = "]}";
	size_t size = strlen(s1) + strlen(s2);
	char *resblock = malloc(sizeof(char));
	size_t blocksize = 0;
	for (size_t i = 0; i < bc->blocksNumber; i++){
		char *blockjson = blockToJson(&bc->blocks[i]);
		size_t t = blocksize + strlen(blockjson) + 1;
		resblock = realloc(resblock, t);
		sprintf(resblock + blocksize,"%s,",blockjson);
		blocksize += strlen(blockjson) + 1;
		free(blockjson);
	}
	char *json = calloc(size + blocksize + 1, sizeof(char));
	sprintf(json, "%s%s%s", s1, resblock, s2);
	free(resblock);
	return json;
}
