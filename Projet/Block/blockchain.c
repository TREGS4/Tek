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

int addBlock(BLOCKCHAIN *blockchain, BLOCK block)
{
	BLOCK lastBlock = *(getLastBlock(blockchain));
	if (block.time < lastBlock.time){
		printf("BLOCK timestamp invalide (prev > current). Nothing have been add in the blockchain\n");
		return 0;
	}
	if (memcmp(block.previusHash, lastBlock.blockHash, SHA256_BLOCK_SIZE) != 0){
		printf("BLOCK invalid. Nothing have been add in the blockchain\n");
		return 0;
	}
	BYTE hash[SHA256_BLOCK_SIZE];
	getHash(&block, hash);
	if (memcmp(hash, block.blockHash, SHA256_BLOCK_SIZE) != 0){
		printf("BLOCK invalid. Nothing have been add in the blockchain\n");
		return 0;
	}
	
	blockchain->blocksNumber += 1;
	
	blockchain->blocks = realloc(blockchain->blocks, blockchain->blocksNumber * sizeof(BLOCK));
	if (blockchain->blocks == NULL)
		errx(1, "Allocation of the new block in the blockchain failed.\n");
	
	blockchain->blocks[blockchain->blocksNumber - 1] = block;	
	return 1;
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

	BYTE sum[4 * SHA256_BLOCK_SIZE + 1 + 11];
	sprintf((char *)sum,"%011ld%s%s", newGenesis.time, (char *)Aprev_hash, (char *)Amerkle_hash);
	ull_t proof = (ull_t)mine_from_string((char *)sum, 1, 2);

	newGenesis.proof = proof;

	BYTE hash[SHA256_BLOCK_SIZE];
	getHash(&newGenesis, hash);

	memcpy(newGenesis.blockHash, hash, SHA256_BLOCK_SIZE);

	return newGenesis;
}


int checkBlockchain(BLOCKCHAIN *blockchain)
{
	for (ull_t i = 1 ; i < blockchain->blocksNumber ; i++)
	{
		BYTE hash[SHA256_BLOCK_SIZE];
		BLOCK current = blockchain->blocks[i];
		BLOCK prev = blockchain->blocks[i - 1];
		getHash(&current, hash);
			
		if (current.time < prev.time)
			return 1;

		if (memcmp(current.previusHash, prev.blockHash, SHA256_BLOCK_SIZE) != 0)
			return 1;
			
		if (memcmp(current.blockHash, hash, SHA256_BLOCK_SIZE) != 0)
			return 1;
	}
	
	return 0;
}

void updateTlWithBc(TRANSACTIONS_LIST *tl, BLOCKCHAIN *bc){
	TRANSACTIONS_LIST new_tl = initListTxs();
	for (ull_t i = 0; i < tl->size; i++){
		if (!findTxsInBc(&tl->transactions[i], bc)){
			if (enoughMoney(tl->transactions[i].sender, tl->transactions[i].amount, bc)){
				addTx(&new_tl, &tl->transactions[i]);
			}
		}
	}
	freeTxsList(tl);
	*tl = new_tl;
}

int findTxsInBc(TRANSACTION *txs, BLOCKCHAIN *bc){
	for (ull_t i = 1; i < bc->blocksNumber; i++){
		for (ull_t a = 0; a < bc->blocks[i].tl.size; a++){
			if (TxsEqual(txs, &bc->blocks[i].tl.transactions[a])){
				return TRUE;
			}
		}
	}
	return FALSE;
	
}

char *blockchainToJson(BLOCKCHAIN *bc)
{
	char *s1 = "{\"blocks\":[";
	char *s2 = "]}";
	ull_t size = strlen(s1) + strlen(s2);
	char *resblock = malloc(sizeof(char));
	ull_t blocksize = 0;
	for (ull_t i = 0; i < bc->blocksNumber; i++){
		char *blockjson = blockToJson(&bc->blocks[i]);
		ull_t t = blocksize + strlen(blockjson) + 1;
		resblock = realloc(resblock, t);
		if (i == bc->blocksNumber - 1){
			sprintf(resblock + blocksize,"%s",blockjson);
			blocksize += strlen(blockjson);
		}else{
			sprintf(resblock + blocksize,"%s,",blockjson);
			blocksize += strlen(blockjson) + 1;
		}
		free(blockjson);
	}
	char *json = calloc(size + blocksize + 1, sizeof(char));
	sprintf(json, "%s%s%s", s1, resblock, s2);
	free(resblock);
	return json;
}


BLOCKCHAIN_BIN blockchainToBin(BLOCKCHAIN *bc)
{
	ull_t nbBlocks = bc->blocksNumber;
	ull_t size = sizeof(ull_t);
	ull_t total_size = size;

	BYTE *res = malloc(size);
	ull_t cursor = 0;
	memcpy(res + cursor, &nbBlocks, sizeof(nbBlocks));
	cursor += sizeof(nbBlocks);


	for (ull_t i = 0; i < bc->blocksNumber; i++){
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
	ull_t nbBlocks;
	ull_t cursor = 0;
	memcpy(&nbBlocks, bin + cursor, sizeof(nbBlocks));
	cursor += sizeof(nbBlocks);
	

	BLOCKCHAIN bc = {
		.blocksNumber = nbBlocks,
	};
	bc.blocks = malloc(sizeof(BLOCK)*nbBlocks);
	
	for (ull_t i = 0; i < nbBlocks; i++){
		ull_t nbTxs;
		memcpy(&nbTxs, bin + cursor, sizeof(nbTxs));
		BLOCK b = binToBlock(bin + cursor);
		cursor += getSizeOf_blockbin(&b);
		bc.blocks[i] = b;
	}
	return bc;
}



void freeBlockchain(BLOCKCHAIN *bc){
	for (ull_t i = 0; i < bc->blocksNumber; i++){
		freeBlock(&bc->blocks[i]);
	}
	free(bc->blocks);
}


ull_t amountMoney(char *address, BLOCKCHAIN *bc)
{
	//N'A PAS ETAIT TESTEE

	ull_t money = 0;
	ull_t lenAddr = strlen(address);

	for(ull_t i = 0; i < bc->blocksNumber; i++)
	{
		for(ull_t j = 0; j < bc->blocks[i].tl.size; j++)
		{
			TRANSACTION temp = bc->blocks[i].tl.transactions[j];

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


int enoughMoney(char *address, ull_t amount, BLOCKCHAIN *bc)
{
	//N'A PAS ETAIT TESTEE

	int res = TRUE;
	if(amountMoney(address, bc) < amount)
		res = FALSE;
	
	return res;
}


void printBlockchain(BLOCKCHAIN blockchain)
{
	printf("-------------------BLOCKCHAIN-------------------\n\n");
	printf("----Genesis----\n");

	printf("Hash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", blockchain.blocks[0].blockHash[i]);
	}
	printf("\n");
	printf("---------------\n\n");

	for (ull_t i = 1; i < blockchain.blocksNumber; i++)
	{
		printf("---------------BLOCK %02llu---------------\n", i);
		printBlock(blockchain.blocks[i]);
		printf("--------------------------------------\n\n");
	}
	printf("------------------------------------------------\n");
}


int saveBlockchain(BLOCKCHAIN blockchain){
	char *filename = "bcsave.data";

    // open the file for writing
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening the file %s", filename);
        return 0;
    }
    // write to the text file
    BLOCKCHAIN_BIN bcbin = blockchainToBin(&blockchain);
	fwrite(&bcbin.nbBytes, sizeof(bcbin.nbBytes), 1, fp);
	fwrite(bcbin.bin, sizeof(BYTE), bcbin.nbBytes, fp);

    // close the file
    fclose(fp);

    return 1;
}

BLOCKCHAIN *loadBlockchain(){
	char *filename = "bcsave.data";
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
    {
        printf("Error: could not open file %s\n", filename);
        return NULL;
    }

	ull_t size = 0;
	fread(&size, sizeof(size), 1, fp);
	
	BYTE *bin = calloc(sizeof(BYTE), size);
	if (bin == NULL){
		return NULL;
	}
    fread(bin, sizeof(BYTE), size, fp);

	BLOCKCHAIN bc1 = binToBlockchain(bin);
	
	BLOCKCHAIN *bc2 = malloc(sizeof(BLOCKCHAIN));
	if (bc2 == NULL){
		return NULL;
	}

	memcpy(bc2, &bc1, sizeof(BLOCKCHAIN));
	fclose(fp);
    return bc2;
}