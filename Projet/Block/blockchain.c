#include "blockchain.h"
#include "block.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include "../Noeud_de_minage/minage.h"


BLOCK *getLastBlock(BLOCKCHAIN *blockchain)
{
	return &(blockchain->blocks[blockchain->blocksNumber - 1]);
}

void addBlock(BLOCKCHAIN *blockchain, BLOCK block)
{
	BLOCK lastBlock = *(getLastBlock(blockchain));
	if (memcmp(block.previusHash, lastBlock.blockHash, SHA256_BLOCK_SIZE) != 0){
		printf("BLOCK mined invalid. Nothing have been add in the blockchain\n");
		return;
	}
	BYTE hash[SHA256_BLOCK_SIZE];
	getHash(&block, hash);
	if (memcmp(hash, block.blockHash, SHA256_BLOCK_SIZE) != 0){
		printf("BLOCK mined invalid. Nothing have been add in the blockchain\n");
		return;
	}
	
	blockchain->blocksNumber += 1;
	
	blockchain->blocks = realloc(blockchain->blocks, blockchain->blocksNumber * sizeof(BLOCK));
	if (blockchain->blocks == NULL)
		errx(1, "Allocation of the new block in the blockchain failed.\n");
	
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
	BLOCK newGenesis = initBlock();

	TRANSACTION txs = CreateTxs(1000, "", "MFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAIVdUtUR9QG0wQl2jf00+0NiTOusk69PGFuHBEAoy7NIIzM7As81H1lGYUIg5pXVrWQ9ACt99trhVWNGRo3VMicCAwEAAQ==");
	addTx(&newGenesis.tl, &txs);
	memset(newGenesis.previusHash, 0, SHA256_BLOCK_SIZE);

	BYTE merkle_hash[SHA256_BLOCK_SIZE];
	getMerkleHash(&newGenesis, merkle_hash);

	char Aprev_hash[2 * SHA256_BLOCK_SIZE + 1];
	char Amerkle_hash[2 * SHA256_BLOCK_SIZE + 1];

	sha256ToAscii(newGenesis.previusHash, Aprev_hash);
	sha256ToAscii(merkle_hash, Amerkle_hash);

	Amerkle_hash[2 * SHA256_BLOCK_SIZE] = '\0';
	Aprev_hash[2 * SHA256_BLOCK_SIZE] = '\0';

	BYTE sum[4 * SHA256_BLOCK_SIZE + 1];
	sprintf((char *)sum,"%s%s", (char *)Aprev_hash, (char *)Amerkle_hash);
	size_t proof = (size_t)mine_from_string((char *)sum, 1, 1);

	newGenesis.proof = proof;

	BYTE hash[SHA256_BLOCK_SIZE];
	getHash(&newGenesis, hash);
	memcpy(newGenesis.blockHash, hash, SHA256_BLOCK_SIZE);

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


BLOCKCHAIN binToBlockchain(BYTE *bin){
	size_t nbBlocks;
	size_t cursor = 0;
	memcpy(&nbBlocks, bin + cursor, sizeof(nbBlocks));
	cursor += sizeof(nbBlocks);

	BLOCKCHAIN bc = {
		.blocksNumber = nbBlocks,
	};
	bc.blocks = malloc(sizeof(BLOCK)*nbBlocks);

	for (size_t i = 0; i < nbBlocks; i++){
		size_t nbTxs;
		memcpy(&nbTxs, bin + cursor, sizeof(nbTxs));
		BLOCK b = binToBlock(bin + cursor);
		cursor += getSizeOf_blockbin(&b);
		bc.blocks[i] = b;
	}
	return bc;
}



void freeBlockchain(BLOCKCHAIN *bc){
	for (size_t i = 0; i < bc->blocksNumber; i++){
		freeBlock(&bc->blocks[i]);
	}
	free(bc->blocks);
}


size_t amountMoney(char *address, BLOCKCHAIN *bc)
{
	//N'A PAS ETAIT TESTEE

	size_t money = 0;
	size_t lenAddr = strlen(address);

	for(size_t i = 0; i < bc->blocksNumber; i++)
	{
		for(size_t j = 0; j < bc->blocks[i].tl.size; j++)
		{
			TRANSACTION temp = bc->blocks->tl.transactions[j];

			if(memcmp(address, temp.sender, lenAddr) == 0)
			{
				money -= temp.amount;
			}
			if(memcmp(address, temp.receiver, lenAddr) == 0)
			{
				money += temp.amount;
			}
		}
	}

	return money;
}


int enoughMoney(char *address, size_t amount, BLOCKCHAIN *bc)
{
	//N'A PAS ETAIT TESTEE

	int res = TRUE;
	if(amountMoney(address, bc) < amount)
		res = FALSE;
	
	return res;
}