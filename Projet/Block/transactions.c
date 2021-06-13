#include "transactions.h"
#include "../Hash/sha256.h"
#include "../general_informations.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

TRANSACTION CreateTxs(ull_t amount, char *sender, char *receiver)
{
	time_t rawtime;
	time(&rawtime);
	TRANSACTION txs = {
		.amount = amount,
		.time = rawtime,
	};
	ull_t sender_size = strlen(sender);
	ull_t receiver_size = strlen(receiver);
	txs.sender = calloc(sender_size + 1, 1);
	txs.receiver = calloc(receiver_size + 1, 1);
	memcpy(txs.sender, sender, sender_size);
	memcpy(txs.receiver, receiver, receiver_size);
	return txs;
}

int TxsEqual(TRANSACTION *t1, TRANSACTION *t2){
	if (t1->amount == t2->amount && t1->time == t2->time){
		if (memcmp(t1->sender, t2->sender, strlen(t1->sender) + 1) == 0){
			if (memcmp(t1->receiver, t2->receiver, strlen(t1->receiver) + 1) == 0){
				return TRUE;
			}
		}
	}
	return FALSE;
}

char *txsToString(TRANSACTION *txs)
{
	ull_t size = strlen(txs->sender) + strlen(txs->receiver) + 14 + 11 + 1;
	char *res = calloc(1, size);
	sprintf(res, "%s%s%014llu%011ld", txs->sender, txs->receiver, txs->amount, txs->time);
	return res;
}

char *tlToString(TRANSACTIONS_LIST *tl)
{
	char *res = calloc(sizeof(char),1);
	ull_t offset = 0;
	for (ull_t i = 0; i < tl->size; i++){
		char *txs_buf = txsToString(&tl->transactions[i]);
		ull_t size = strlen(txs_buf);
		res = realloc(res, offset + size + 1);
		sprintf(res + offset, "%s", txs_buf);
		offset += size;
	}
	return res;
}

char *txsToJson(TRANSACTION *t)
{
	char *s1 = "{\"sender\":\"";
	char *s2 = "\",\"receiver\":\"";
	char *s3 = "\",\"amount\":";
	char *s4 = ",\"time\":";
	char *s5 = "}";
	ull_t txs_size = strlen(t->receiver) + strlen(t->sender) + 14 + 11;
	ull_t size = strlen(s1) + strlen(s2) + strlen(s3) + strlen(s4) + strlen(s5);
	char *res = calloc(size + txs_size + 1, sizeof(char));
	char *json = NULL;
	if (res != NULL)
	{
		sprintf(res, "%s%s%s%s%s%llu%s%ld%s", s1, t->sender, s2, t->receiver, s3, t->amount, s4, t->time, s5);
		size = strlen(res);
		json = calloc(size + 1, sizeof(char));
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
	ull_t size = strlen(s1) + strlen(s2);

	char *json = NULL;

	ull_t nbTxs = tl->size;
	char *restxs = NULL;
	ull_t txssize = 0;
	for (ull_t i = 0; i < nbTxs; i++)
	{
		char *txsjson = txsToJson(&tl->transactions[i]);
		ull_t size_json = strlen(txsjson);
		ull_t t = txssize + size_json + 1;
		restxs = realloc(restxs, t + 1);
		if (restxs != NULL)
		{
			if (i == nbTxs - 1){
				sprintf(restxs + txssize, "%s", txsjson);
				txssize += size_json;
			}else{
				sprintf(restxs + txssize, "%s,", txsjson);
				txssize += size_json + 1;
			}
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
		
	return json;
}

ull_t getSizeOf_txsbin(TRANSACTION *t)
{
	ull_t size = strlen(t->sender) + strlen(t->receiver);
	size += sizeof(ull_t) * 2;
	size += sizeof(t->amount) + sizeof(t->time);
	size += sizeof(ull_t);
	return size;
}
TRANSACTION_BIN txsToBin(TRANSACTION *t)
{
	ull_t size = getSizeOf_txsbin(t) - sizeof(ull_t);
	TRANSACTION_BIN txsbin = {
		.nbBytes = size + sizeof(ull_t),
	};
	txsbin.bin = calloc(1, txsbin.nbBytes);
	ull_t offset = 0;

	ull_t tmp_size = sizeof(ull_t);
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
	ull_t offset = 0;

	ull_t size;
	memcpy(&size, bin, sizeof(ull_t));
	offset += sizeof(ull_t);

	ull_t sender_size;
	memcpy(&sender_size, bin + offset, sizeof(ull_t));
	offset += sizeof(ull_t);
	txs.sender = calloc(1, sender_size);
	memcpy(txs.sender, bin + offset, sender_size);
	offset += sender_size;

	ull_t receiver_size;
	memcpy(&receiver_size, bin + offset, sizeof(ull_t));
	offset += sizeof(ull_t);
	txs.receiver = calloc(1, receiver_size);
	memcpy(txs.receiver, bin + offset, receiver_size);
	offset += receiver_size;

	ull_t amount_size = sizeof(txs.amount);
	memcpy(&txs.amount, bin + offset, amount_size);
	offset += amount_size;


	ull_t time_size = sizeof(txs.time);
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
	TRANSACTION newtxs = CreateTxs(t->amount, t->sender, t->receiver);
	newtxs.time = t->time;
	tl->transactions[tl->size] = newtxs;
	tl->size += 1;
}

int hasSendedTxs(char *address, TRANSACTIONS_LIST *tl){
	ull_t size_address = strlen(address);
	for (ull_t i = 0; i < tl->size; i++){
		ull_t size_sender = strlen(tl->transactions[i].sender);
		ull_t cmp_size = (size_address > size_sender) ? size_address : size_sender;
		if (memcmp(tl->transactions[i].sender, address, cmp_size) == 0){
			return 1;
		}
	}
	return 0;
}

void removeTxsList(TRANSACTIONS_LIST *tl, ull_t start, ull_t end){
	TRANSACTIONS_LIST new_tl = initListTxs();
	for (ull_t i = 0; i < start; i++){
		addTx(&new_tl, &tl->transactions[i]);
	}
	for (ull_t i = end; i < tl->size; i++){
		addTx(&new_tl, &tl->transactions[i]);
	}
	freeTxsList(tl);
	*tl = new_tl;
}

void clearTxsList(TRANSACTIONS_LIST *tl){
	freeTxsList(tl);
	*tl = initListTxs();
}

void freeTxsList(TRANSACTIONS_LIST *tl)
{
	for (ull_t i = 0; i < tl->size; i++){
		freeTxs(&tl->transactions[i]);
	}
	
	free(tl->transactions);
}


void freeTxs(TRANSACTION *txs){
	free(txs->sender);
	free(txs->receiver);
}