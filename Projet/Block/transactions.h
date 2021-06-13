#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdlib.h>
#include <time.h>
#include "../Hash/sha256.h"
#include "./account.h"
#include "../general_informations.h"


typedef struct
{
	char *sender;
	char *receiver;
	ull_t amount;
	time_t time;
} TRANSACTION;

typedef struct
{
	TRANSACTION *transactions;
	ull_t size;
	ull_t capacity;
} TRANSACTIONS_LIST;


typedef struct
{
	BYTE *bin;
	ull_t nbBytes;
} TRANSACTION_BIN;

typedef struct {
    TRANSACTIONS_LIST tl;
    pthread_mutex_t mutex;
} TL_M;




TRANSACTION CreateTxs(ull_t amount, char* sender, char* receiver);
int TxsEqual(TRANSACTION *t1, TRANSACTION *t2);
char *txsToString(TRANSACTION *txs);
char *tlToString(TRANSACTIONS_LIST *tl);
char *txsToJson(TRANSACTION *t);
char *tlToJson(TRANSACTIONS_LIST *tl);
TRANSACTION_BIN txsToBin(TRANSACTION *t);
TRANSACTION binToTxs(BYTE *bin);
ull_t getSizeOf_txsbin(TRANSACTION *t);
TRANSACTIONS_LIST initListTxs();
void addTx(TRANSACTIONS_LIST *tl, TRANSACTION *t);
int hasSendedTxs(char *address, TRANSACTIONS_LIST *tl);
void clearTxsList(TRANSACTIONS_LIST *tl);
void removeTxsList(TRANSACTIONS_LIST *tl, ull_t start, ull_t end); //inclusive -  exclusive
void freeTxsList(TRANSACTIONS_LIST *tl);
void freeTxs(TRANSACTION *txs);



#endif
