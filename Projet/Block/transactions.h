#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <stdlib.h>

#define NB_TRANSACTIONS_PER_BLOCK 10

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


void txsToString(TRANSACTION *txs, char buf[TRANSACTION_SIZE]);
char *txsToJson(TRANSACTION *t);
TRANSACTIONS_LIST initListTxs();
void addTx(TRANSACTIONS_LIST *tl, TRANSACTION *t);
void clearTxsList(TRANSACTIONS_LIST *tl);



#endif
