#include "block.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


//len of the proof if it was a string
size_t len_of_proof(size_t proof){
	size_t len = 0;
	size_t tmp = proof;
	while(tmp != 0){
		tmp /= 10;
		len++;
	}
	return len;
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
	size_t proof_size = len_of_proof(b->proof);
	BYTE buf[4 * SHA256_BLOCK_SIZE + proof_size + 1];
	BYTE merkleHash[SHA256_BLOCK_SIZE];
	getMerkleHash(b, merkleHash);

	char var1[SHA256_BLOCK_SIZE*2];
	char var2[SHA256_BLOCK_SIZE*2];
	sha256ToAscii(b->previusHash, var1);
	sha256ToAscii(merkleHash, var2);

	size_t offset = 0;
	sprintf((char*)buf + offset, "%ld", b->proof);
	offset += proof_size;
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
	char *s4 = ",\"transactions\":[";
	char *s5 = "]}";
	size_t size = strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4) + strlen(s5);
	size += SHA256_BLOCK_SIZE * 2 * 2 + len_of_proof(b->proof);


	char var1[SHA256_BLOCK_SIZE*2+1];
	char var2[SHA256_BLOCK_SIZE*2+1];
	sha256ToAscii(b->previusHash, var1);
	sha256ToAscii(b->blockHash, var2);
	var1[SHA256_BLOCK_SIZE*2] = 0;
	var2[SHA256_BLOCK_SIZE*2] = 0;

	char *json = NULL;

	size_t nbTxs = b->tl.size;
	char *restxs = NULL;
	size_t txssize = 0;
	for (size_t i = 0; i < nbTxs; i++){
		char *txsjson = txsToJson(&b->tl.transactions[i]);
		size_t t = txssize + strlen(txsjson) + 1;
		restxs = realloc(restxs, t);
		sprintf(restxs + txssize,"%s,",txsjson);
		txssize += strlen(txsjson) + 1;
		free(txsjson);
	}
	json = calloc(size + txssize + 1, sizeof(char));
	if (restxs != NULL){
		sprintf(json, "%s%s%s%s%s%ld%s%s%s", s1, var1, s2, var2, s3, b->proof, s4, restxs, s5);
		free(restxs);
	}else{
		sprintf(json, "%s%s%s%s%s%ld%s%s", s1, var1, s2, var2, s3, b->proof, s4, s5);
	}

	return json;
}


size_t getSizeOf_blockbin(BLOCK *b){
	size_t size = SHA256_BLOCK_SIZE * 2;
	size += len_of_proof(b->proof);
	size += sizeof(size_t);
	for (size_t i = 0; i < b->tl.size; i++){
		size += getSizeOf_txsbin(&b->tl.transactions[i]);
	}
	return size;
}

BLOCK_BIN blockToBin(BLOCK *b)
{
	size_t nbTxs = b->tl.size;
	size_t size = getSizeOf_blockbin(b);

	BYTE *res = malloc(size);
	size_t cursor = 0;
	memcpy(res + cursor, &nbTxs, sizeof(nbTxs));
	cursor += sizeof(nbTxs);
	memcpy(res + cursor, b->previusHash, SHA256_BLOCK_SIZE);
	cursor += SHA256_BLOCK_SIZE;
	memcpy(res + cursor, b->blockHash, SHA256_BLOCK_SIZE);
	cursor += SHA256_BLOCK_SIZE;
	memcpy(res + cursor, &b->proof, sizeof(b->proof));
	cursor += sizeof(b->proof);

	for (size_t i = 0; i < nbTxs; i++)
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
	size_t nbTxs;
	size_t cursor = 0;
	memcpy(&nbTxs, bin + cursor, sizeof(nbTxs));
	cursor += sizeof(nbTxs);
	BYTE previusHash[SHA256_BLOCK_SIZE];
	memcpy(previusHash, bin + cursor, SHA256_BLOCK_SIZE);
	cursor +=  SHA256_BLOCK_SIZE;
	BYTE blockHash[SHA256_BLOCK_SIZE];
	memcpy(blockHash, bin + cursor, SHA256_BLOCK_SIZE);
	cursor +=  SHA256_BLOCK_SIZE;
	size_t proof = 0;
	memcpy(&proof, bin + cursor, sizeof(proof));
	cursor +=  sizeof(proof);

	TRANSACTIONS_LIST tl = initListTxs();
	for (size_t i = 0; i < nbTxs; i++)
	{
		TRANSACTION t = binToTxs(bin + cursor);
		cursor += getSizeOf_txsbin(&t);
		addTx(&tl, &t);
	}
	BLOCK b = {
		.tl = tl,
		.proof = proof,
	};
	memcpy(b.previusHash, previusHash, SHA256_BLOCK_SIZE);
	memcpy(b.blockHash, blockHash, SHA256_BLOCK_SIZE);
	return b;
}