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
	newGenesis.tl = initListTxs();
	
	return newGenesis;
}


int checkBlockchain(BLOCKCHAIN *blockchain)
{
	for (size_t i = 1 ; i < blockchain->blocksNumber ; i++)
	{
		BYTE hash[SHA256_BLOCK_SIZE];
		BLOCK current = blockchain->blocks[i];
		BLOCK prev = blockchain->blocks[i - 1];
		getHash(&current, hash);
		
		if (memcmp(current.previusHash, prev.blockHash, SHA256_BLOCK_SIZE) != 0)
			return 1;
			
		if (memcmp(current.blockHash, hash, SHA256_BLOCK_SIZE) != 0)
			return 1;
	}
	
	return 0;
}


char *blockchainToJson(BLOCKCHAIN *bc)
{
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


BLOCKCHAIN_BIN blockchainToBin(BLOCKCHAIN *bc)
{
	size_t nbBlocks = bc->blocksNumber;
	size_t size = sizeof(size_t);
	size_t total_size = size;

	BYTE *res = malloc(size);
	size_t cursor = 0;
	memcpy(res + cursor, &nbBlocks, sizeof(nbBlocks));
	cursor += sizeof(nbBlocks);

	for (size_t i = 0; i < bc->blocksNumber; i++){
		BLOCK_BIN blockbin = blockToBin(&bc->blocks[i]);
		total_size += blockbin.nbBytes;
		res = realloc(res, total_size);
		memcpy(res + cursor, blockbin.bin, blockbin.nbBytes);
		cursor += blockbin.nbBytes;
		free(blockbin.bin);
	}
	BLOCKCHAIN_BIN bcbin = {
		.bin = res,
		.nbBytes = total_size,
	};
	return bcbin;
}
