#include "block.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void txsToString(TRANSACTION *txs, char buf[TRANSACTION_SIZE])
{
	sprintf(buf, "%s%s%014d", txs->sender, txs->receiver, txs->amount);
}

void getHash(BLOCK *b, BYTE hash[SHA256_BLOCK_SIZE])
{
	BYTE buf[4 * SHA256_BLOCK_SIZE + 1];
	BYTE merkleHash[SHA256_BLOCK_SIZE];
	getMerkleHash(b, merkleHash);

	char var1[SHA256_BLOCK_SIZE*2];
	char var2[SHA256_BLOCK_SIZE*2];
	sha256ToAscii(b->previusHash, var1);
	sha256ToAscii(merkleHash, var2);

	memcpy(buf, var1, SHA256_BLOCK_SIZE*2);
	memcpy(buf + SHA256_BLOCK_SIZE *2, var2, SHA256_BLOCK_SIZE*2);
	buf[SHA256_BLOCK_SIZE * 4] = 0;

	sha256(buf, hash);
}

void getMerkleHash(BLOCK *b, BYTE merkleHash[SHA256_BLOCK_SIZE])
{
	BYTE buf[TRANSACTION_SIZE*NB_TRANSACTIONS_PER_BLOCK];
	size_t offset = 0;
	for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++){
		char txs_buf[TRANSACTION_SIZE];
		txsToString(&(b->transactions[i]), txs_buf);
		sprintf((char*)buf+offset, "%s", txs_buf);
		offset += strlen(txs_buf);
	}

	sha256(buf, merkleHash);
}



char *txsToJson(TRANSACTION *t){
	char *s1 = "{\"sender\":\"";
	char *s2 = "\",\"receiver\":\"";
	char *s3 = "\",\"amount\":";
	char *s4 = "}";
	size_t size = strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4);
	char *res = calloc(size + TRANSACTION_SIZE + 1, sizeof(char));
	sprintf(res, "%s%s%s%s%s%d%s", s1, t->sender,s2, t->receiver, s3, t->amount, s4);	
	char *json = calloc(strlen(res), sizeof(char));
	memcpy(json, res, strlen(res));
	free(res);
	return json;
}
char *blockToJson(BLOCK *b){
	char *s1 = "{\"previousHash\":\"";
	char *s2 = "\",\"currentHash\":\"";
	char *s3 = "\",\"transactions\":[";
	char *s4 = "]}";
	size_t size = strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4);
	size += SHA256_BLOCK_SIZE * 2 * 2;

	int isGenesis = 1;
	int a = 0;
	while (isGenesis && a < SHA256_BLOCK_SIZE){
		if (b->previusHash[a] != 0){
			isGenesis = 0;
		}
		a++;
	}

	char var1[SHA256_BLOCK_SIZE*2+1];
	char var2[SHA256_BLOCK_SIZE*2+1];
	sha256ToAscii(b->previusHash, var1);
	sha256ToAscii(b->blockHash, var2);
	var1[SHA256_BLOCK_SIZE*2] = 0;
	var2[SHA256_BLOCK_SIZE*2] = 0;

	char *json = NULL;

	if (!isGenesis){
		char *restxs = NULL;
		size_t txssize = 0;
		for (int i = 0; i < NB_TRANSACTIONS_PER_BLOCK; i++){
			char *txsjson = txsToJson(&b->transactions[i]);
			size_t t = txssize + strlen(txsjson) + 1;
			restxs = realloc(restxs, t);
			sprintf(restxs + txssize,"%s,",txsjson);
			txssize += strlen(txsjson) + 1;
			free(txsjson);
		}
		json = calloc(size + txssize + 1, sizeof(char));
		sprintf(json, "%s%s%s%s%s%s%s", s1, var1, s2, var2, s3, restxs, s4);
		free(restxs);
	}else{
		json = calloc(size + 1, sizeof(char));
		sprintf(json, "%s%s%s%s%s%s", s1, var1, s2, var2, s3, s4);
	}
	return json;
}
