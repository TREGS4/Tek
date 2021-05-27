#include "transactions.h"
#include "../Hash/sha256.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

TRANSACTION CreateTxs(size_t amount, char *sender, char *receiver)
{
	time_t rawtime;
	time(&rawtime);
	TRANSACTION txs = {
		.amount = amount,
		.time = rawtime,
	};
	size_t sender_size = strlen(sender);
	size_t receiver_size = strlen(receiver);
	txs.sender = calloc(sender_size, 1);
	txs.receiver = calloc(receiver_size, 1);
	memcpy(txs.sender, sender, sender_size);
	memcpy(txs.receiver, receiver, receiver_size);
	return txs;
}

char *txsToString(TRANSACTION *txs)
{
	size_t size = strlen(txs->sender) + strlen(txs->receiver) + 14 + 11 + 1;
	char *res = calloc(1, size);
	sprintf(res, "%s%s%014ld%011ld", txs->sender, txs->receiver, txs->amount, txs->time);
	return res;
}

char *txsToJson(TRANSACTION *t)
{
	char *s1 = "{\"sender\":\"";
	char *s2 = "\",\"receiver\":\"";
	char *s3 = "\",\"amount\":";
	char *s4 = "\",\"time\":";
	char *s5 = "}";
	size_t txs_size = strlen(t->receiver) + strlen(t->sender) + 14 + 11;
	size_t size = strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4) + strlen(s5);
	char *res = calloc(size + txs_size + 1, sizeof(char));
	char *json = NULL;
	if (res != NULL)
	{
		sprintf(res, "%s%s%s%s%s%ld%s%ld%s", s1, t->sender, s2, t->receiver, s3, t->amount, s4, t->time, s5);
		size = strlen(res);
		json = calloc(size, sizeof(char));
		if(json != NULL)
		{
			memcpy(json, res, size);
		}
		else
		{
			printf("TU FAIS TJRS DE LA MDERDE\n");
		}
		
		free(res);
	}
	else
	{
		printf("TU FAIS DE LA MERDE ADRIEN\n");
	}

	return json;
}
char *tlToJson(TRANSACTIONS_LIST *tl)
{
	char *s1 = "{\"transactions\":[";
	char *s2 = "]}";
	size_t size = strlen(s1) + strlen(s2);

	char *json = NULL;

	size_t nbTxs = tl->size;
	char *restxs = NULL;
	size_t txssize = 0;
	for (size_t i = 0; i < nbTxs; i++)
	{
		printf("%lu\n", i);
		char *txsjson = txsToJson(&tl->transactions[i]);
		size_t size_json = strlen(txsjson);
		size_t t = txssize + size_json + 1;
		restxs = realloc(restxs, t);
		if (restxs != NULL)
		{
			sprintf(restxs + txssize, "%s,", txsjson);
			txssize += size_json + 1;
			free(txsjson);
		}
		else
			printf("PROBLEM WITHE THE REALLOC IN RESTXS\n");
	}
	json = calloc(size + txssize + 1, sizeof(char) * 2);

	if (json != NULL)
	{

		if (restxs != NULL)
		{
			sprintf(json, "%s%s%s", s1, restxs, s2);
			free(restxs);
		}
		else
		{
			sprintf(json, "%s%s", s1, s2);
		}
	}
	else
		printf("PROBLEMN WITH CALLOC JSON");
		
	return restxs;
}

size_t getSizeOf_txsbin(TRANSACTION *t)
{
	size_t size = strlen(t->sender) + strlen(t->receiver);
	size += sizeof(size_t) * 2;
	size += sizeof(t->amount) + sizeof(t->time);
	size += sizeof(size_t);
	return size;
}
TRANSACTION_BIN txsToBin(TRANSACTION *t)
{
	size_t size = getSizeOf_txsbin(t) - sizeof(size_t);
	TRANSACTION_BIN txsbin = {
		.nbBytes = size + sizeof(size_t),
	};
	txsbin.bin = calloc(1, txsbin.nbBytes);
	size_t offset = 0;

	size_t tmp_size = sizeof(size_t);
	memcpy(txsbin.bin, &size, tmp_size);
	offset += tmp_size;

	tmp_size = strlen(t->sender);
	memcpy(txsbin.bin + offset, &tmp_size, sizeof(tmp_size));
	offset += sizeof(tmp_size);
	memcpy(txsbin.bin + offset, t->sender, tmp_size);
	offset += tmp_size;

	tmp_size = strlen(t->receiver);
	memcpy(txsbin.bin + offset, &tmp_size, sizeof(tmp_size));
	offset += sizeof(tmp_size);
	memcpy(txsbin.bin + offset, t->receiver, tmp_size);
	offset += tmp_size;

	tmp_size = sizeof(t->amount);
	memcpy(txsbin.bin + offset, &t->amount, tmp_size);
	offset += tmp_size;

	tmp_size = sizeof(t->time);
	memcpy(txsbin.bin + offset, &t->time, tmp_size);
	offset += tmp_size;

	return txsbin;
}

TRANSACTION binToTxs(BYTE *bin)
{
	TRANSACTION txs;
	size_t offset = 0;

	size_t size;
	memcpy(&size, bin, sizeof(size_t));
	offset += sizeof(size_t);

	size_t sender_size;
	memcpy(&sender_size, bin + offset, sizeof(size_t));
	offset += sizeof(size_t);
	txs.sender = calloc(1, sender_size);
	memcpy(txs.sender, bin + offset, sender_size);
	offset += sender_size;

	size_t receiver_size;
	memcpy(&receiver_size, bin + offset, sizeof(size_t));
	offset += sizeof(size_t);
	txs.receiver = calloc(1, receiver_size);
	memcpy(txs.receiver, bin + offset, receiver_size);
	offset += receiver_size;

	size_t amount_size = sizeof(txs.amount);
	memcpy(&txs.amount, bin + offset, amount_size);
	offset += amount_size;

	size_t time_size = sizeof(txs.time);
	memcpy(&txs.time, bin + offset, time_size);
	offset += time_size;

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
	{
		printf("error initListTxs\n");
		exit(1);
	}

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
