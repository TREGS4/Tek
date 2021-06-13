#include "block.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../general_informations.h"


BLOCK initBlock(){
	BLOCK b;
	time_t rawtime;
	time(&rawtime);
	b.time = rawtime;
	b.tl = initListTxs();
	return b;
}

void freeBlock(BLOCK *b){
	freeTxsList(&b->tl);
}

bool isGenesis(BLOCK *b){
	int isGenesis = true;
	int a = 0;
	while (isGenesis && a < SHA256_BLOCK_SIZE){
		if (b->previusHash[a] != 0){
			isGenesis = false;
		}
		a++;
	}
	return isGenesis;
}

void getHash(BLOCK *b, BYTE hash[SHA256_BLOCK_SIZE])
{
	ull_t proof_size = len_of_proof(b->proof);
	BYTE buf[4 * SHA256_BLOCK_SIZE + proof_size + 1 + 11];
	BYTE merkleHash[SHA256_BLOCK_SIZE];
	getMerkleHash(b, merkleHash);

	char var1[SHA256_BLOCK_SIZE*2];
	char var2[SHA256_BLOCK_SIZE*2];
	sha256ToAscii(b->previusHash, var1);
	sha256ToAscii(merkleHash, var2);

	ull_t offset = 0;
	sprintf((char*)buf + offset, "%llu", b->proof);
	offset += proof_size;
	sprintf((char*)buf + offset, "%011llu", (ull_t)b->time);
	offset += 11;
	memcpy(buf + offset, var1, SHA256_BLOCK_SIZE*2);
	offset += SHA256_BLOCK_SIZE*2;
	memcpy(buf + offset, var2, SHA256_BLOCK_SIZE*2);
	offset += SHA256_BLOCK_SIZE*2;

	buf[offset] = 0;

	sha256(buf, hash);
}

void getMerkleHash(BLOCK *b, BYTE merkleHash[SHA256_BLOCK_SIZE])
{
	char *buf = tlToString(&b->tl);
	sha256((BYTE*)buf, merkleHash);
	free(buf);
}


char *blockToJson(BLOCK *b)
{
	char *s1 = "{\"previousHash\":\"";
	char *s2 = "\",\"currentHash\":\"";
	char *s3 = "\",\"proof\":";
	char *s4 = ",\"time\":";
	char *s5 = ",\"transactions\":[";
	char *s6 = "]}";
	ull_t size = strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4) + strlen(s5) + strlen(s6);
	size += SHA256_BLOCK_SIZE * 2 * 2 + len_of_proof(b->proof) + 11;


	char var1[SHA256_BLOCK_SIZE*2+1];
	char var2[SHA256_BLOCK_SIZE*2+1];
	sha256ToAscii(b->previusHash, var1);
	sha256ToAscii(b->blockHash, var2);
	var1[SHA256_BLOCK_SIZE*2] = 0;
	var2[SHA256_BLOCK_SIZE*2] = 0;

	char *json = NULL;

	ull_t nbTxs = b->tl.size;
	char *restxs = NULL;
	ull_t txssize = 0;
	for (ull_t i = 0; i < nbTxs; i++){
		char *txsjson = txsToJson(&b->tl.transactions[i]);
		ull_t t = txssize + strlen(txsjson) + 1;
		restxs = realloc(restxs, t);
		if (i == nbTxs - 1){
			sprintf(restxs + txssize,"%s",txsjson);
			txssize += strlen(txsjson);
		}else{
			sprintf(restxs + txssize,"%s,",txsjson);
			txssize += strlen(txsjson) + 1;
		}
		free(txsjson);
	}
	json = calloc(size + txssize + 1, sizeof(char));
	if (restxs != NULL){
		sprintf(json, "%s%s%s%s%s%lld%s%ld%s%s%s", s1, var1, s2, var2, s3, b->proof, s4, b->time, s5, restxs, s6);
		free(restxs);
	}else{
		sprintf(json, "%s%s%s%s%s%lld%s%ld%s%s", s1, var1, s2, var2, s3, b->proof, s4, b->time, s5, s6);
	}

	return json;
}


ull_t getSizeOf_blockbin(BLOCK *b){
	ull_t size = SHA256_BLOCK_SIZE * 2;
	size += sizeof(b->proof);
	size += sizeof(ull_t);
	size += sizeof(ull_t);
	for (ull_t i = 0; i < b->tl.size; i++){
		size += getSizeOf_txsbin(&b->tl.transactions[i]);
	}
	return size;
}

BLOCK_BIN blockToBin(BLOCK *b)
{
	ull_t nbTxs = b->tl.size;
	ull_t size = getSizeOf_blockbin(b);

	BYTE *res = malloc(size);
	ull_t cursor = 0;
	memcpy(res + cursor, &nbTxs, sizeof(nbTxs));
	cursor += sizeof(nbTxs);
	memcpy(res + cursor, b->previusHash, SHA256_BLOCK_SIZE);
	cursor += SHA256_BLOCK_SIZE;
	memcpy(res + cursor, b->blockHash, SHA256_BLOCK_SIZE);
	cursor += SHA256_BLOCK_SIZE;
	memcpy(res + cursor, &b->proof, sizeof(b->proof));
	cursor += sizeof(b->proof);
	ull_t time = (ull_t)b->time;
	memcpy(res + cursor, &time, sizeof(time));
	cursor += sizeof(b->time);

	for (ull_t i = 0; i < nbTxs; i++)
	{
		TRANSACTION_BIN txsbin = txsToBin(&b->tl.transactions[i]);
		memcpy(res + cursor, txsbin.bin, txsbin.nbBytes);
		cursor += txsbin.nbBytes;
		free(txsbin.bin);
	}
	
	BLOCK_BIN blockbin = {
		.bin = res,
		.nbBytes = size,
	};
	return blockbin;
}

BLOCK binToBlock(BYTE *bin){
	ull_t nbTxs;
	ull_t cursor = 0;
	memcpy(&nbTxs, bin + cursor, sizeof(nbTxs));
	cursor += sizeof(nbTxs);
	BYTE previusHash[SHA256_BLOCK_SIZE];
	memcpy(previusHash, bin + cursor, SHA256_BLOCK_SIZE);
	cursor += SHA256_BLOCK_SIZE;
	BYTE blockHash[SHA256_BLOCK_SIZE];
	memcpy(blockHash, bin + cursor, SHA256_BLOCK_SIZE);
	cursor += SHA256_BLOCK_SIZE;
	ull_t proof = 0;
	memcpy(&proof, bin + cursor, sizeof(proof));
	cursor += sizeof(proof);
	ull_t time = 0;
	memcpy(&time, bin + cursor, sizeof(time));
	cursor += sizeof(time);

	TRANSACTIONS_LIST tl = initListTxs();
	for (ull_t i = 0; i < nbTxs; i++)
	{
		TRANSACTION t = binToTxs(bin + cursor);
		cursor += getSizeOf_txsbin(&t);
		addTx(&tl, &t);
	}
	BLOCK b = {
		.tl = tl,
		.proof = proof,
		.time = (time_t)time,
	};
	memcpy(b.previusHash, previusHash, SHA256_BLOCK_SIZE);
	memcpy(b.blockHash, blockHash, SHA256_BLOCK_SIZE);
	return b;
}


void printBlock(BLOCK block)
{
	printf("prevHash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", block.previusHash[i]);
	}
	printf("\n");

	printf("currHash : ");
	for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
	{
		printf("%02x", block.blockHash[i]);
	}
	printf("\n");

	size_t nbTxs = block.tl.size;
	for (size_t i = 0; i < nbTxs; i++)
	{
		printTransaction(block.tl.transactions[i]);
	}
}