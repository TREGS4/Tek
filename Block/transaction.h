#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "../Hash/sha256.h"

#define TRANSACTION_SIZE 

typedef struct
{
	BYTE sender[20];
	BYTE receiver[20];
	int amount;
} TRANSACTION;

const char *TXS_toString(TRANSACTION txs);
#endif
