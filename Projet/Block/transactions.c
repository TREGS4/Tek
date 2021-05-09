#include "transactions.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void txsToString(TRANSACTION *txs, char buf[TRANSACTION_SIZE])
{
	sprintf(buf, "%s%s%014d", txs->sender, txs->receiver, txs->amount);
}

char *txsToJson(TRANSACTION *t)
{
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



size_t getSizeOf_txsbin(){
	return sizeof(TRANSACTION);
}
TRANSACTION_BIN txsToBin(TRANSACTION *t)
{
	TRANSACTION *res = calloc(1, getSizeOf_txsbin());
	*res = *t;
	TRANSACTION_BIN txsbin = {
		.bin = (BYTE*)res,
		.nbBytes = getSizeOf_txsbin(),
	};
	return txsbin;
}

TRANSACTION binToTxs(BYTE *bin){
	TRANSACTION txs;
	memcpy(&txs, bin, getSizeOf_txsbin());
	return txs;
}

TRANSACTIONS_LIST initListTxs()
{
	TRANSACTIONS_LIST newTransactionsList = 
	{
		.transactions = NULL,
		.size = 0,
		.capacity = 1,
	};
	
	newTransactionsList.transactions = malloc(sizeof(TRANSACTION) * newTransactionsList.capacity);
	if (newTransactionsList.transactions == NULL)
		exit(1);

	return newTransactionsList;
}

void addTx(TRANSACTIONS_LIST *tl, TRANSACTION *t)
{
	if (tl->size >= tl->capacity)
	{
		tl->capacity *= 2;
		tl->transactions = realloc(tl->transactions, sizeof(TRANSACTION) * tl->capacity);
		if (tl->transactions == NULL)
			exit(1);
	}

	tl->transactions[tl->size] = *t;
	tl->size += 1;
}

void clearTxsList(TRANSACTIONS_LIST *tl)
{
	free(tl->transactions);
	*tl = initListTxs();
}
