#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdlib.h>
#include "../Hash/sha256.h"
#include "./account.h"

#define TRANSACTION_USER_SIZE 20
#define TRANSACTION_AMOUNT_SIZE 24
#define TRANSACTION_SIZE 2 * TRANSACTION_USER_SIZE + TRANSACTION_AMOUNT_SIZE

typedef struct
{
	char sender[TRANSACTION_USER_SIZE];
	char receiver[TRANSACTION_USER_SIZE];
	int amount;
} TRANSACTION;

typedef struct
{
	TRANSACTION *transactions;
	size_t size;
	size_t capacity;
} TRANSACTIONS_LIST;


typedef struct
{
	BYTE *bin;
	size_t nbBytes;
} TRANSACTION_BIN;

typedef struct {
    TRANSACTIONS_LIST tl;
    pthread_mutex_t mutex;
} TL_M;


void txsToString(TRANSACTION *txs, char buf[TRANSACTION_SIZE]);
char *txsToJson(TRANSACTION *t);
char *tlToJson(TRANSACTIONS_LIST *tl);
TRANSACTION_BIN txsToBin(TRANSACTION *t);
TRANSACTION binToTxs(BYTE *bin);
size_t getSizeOf_txsbin();
TRANSACTIONS_LIST initListTxs();
void addTx(TRANSACTIONS_LIST *tl, TRANSACTION *t);
void clearTxsList(TRANSACTIONS_LIST *tl);



#endif
